#include "level_handler.hpp"
#include "engine/core.hpp"
#include "engine/log.hpp"

#include "engine/input/input_manager.hpp"
#include "engine/renderer/scene_composer.hpp"
#include "engine/app.hpp"

#include "brick.hpp"
#include "graphics_mode.hpp"

#include <imgui.h>
#include "imgui_extensions.hpp"

#include <algorithm>

constexpr float k_PADDLE_VELOCITY = 50.0f;
constexpr float k_BALL_TERMINAL_VELOCITY = 100.0f;
constexpr float k_BALL_NORMAL_SPEED = 35.0f;
// constexpr float k_BALL_LAUNCH_VELOCITY = 10000.0f;
constexpr float k_BALL_LAUNCH_VELOCITY_MIN = 16.0f;
constexpr float k_BALL_LAUNCH_VELOCITY_MAX = k_BALL_NORMAL_SPEED;
constexpr float k_MAX_SPACE_HELD_TIME_SECONDS = 0.8f;
static float k_LAUNCH_SPEED_SCALE = 50.0f;

static float k_BALL_PARTICLE_FREQUENCY = 0.018f;

// conversion constants so that box2d works at 100u = 1m
constexpr float k_UNITS_TO_BOX2D_SCALE = 100.0f;
constexpr float k_BOX2D_TO_UNITS_SCALE = 0.01f;

constexpr float k_PADDLE_WIDTH = 10.0f;

constexpr float k_RAMP_ANGLE = 28.024f;

static float k_FLIPPER_ANGLE_NEUTRAL = -30.0f;
static float k_FLIPPER_ANGLE_MAX = 30.0f;
static float k_FLIPPER_LAUNCH_VELOCITY = 5000.0f;
static float k_FLIPPER_GRAVITY = -9.80f;
static float k_FLIPPER_KICK_DURATION_SECONDS = 1.25f;

static float k_CAMERA_ANGLE_TILT_MAX = 5.0f;

constexpr uint32_t k_BRICKS_COLUMNS = 10;
constexpr uint32_t k_BRICKS_ROWS = 4;

constexpr uint32_t k_MAX_LEVELS = 20;

// for nicer camera motion
// https://easings.net/#easeInOutQuad
float easeInOutQuad(float x) {
    return x < 0.5f ? 2.0f * x * x : 1.0f - pow(-2 * x + 2, 2.0f) / 2.0f;
}

// for RNG stuff
float randomFloat() {
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

void LevelHandler::start() {
    // get pointers to entities in the scene
    m_ballEntity = getEntity()->parent->findNamedEntity("Ball");
    m_paddleEntity = getEntity()->parent->findNamedEntity("Paddle");
    m_cameraEntity = getEntity()->parent->findNamedEntity("Camera");
    m_bricksEntityRoot = getEntity()->parent->findNamedEntity("BrickContainer");

    // get ref to the main shader
    m_shader = ((render::MeshRenderer*)m_ballEntity->findComponent(render::ComponentType::MeshRenderer))->material.shader;
    m_ballParticleSystem = ((render::ParticleSystem*)m_ballEntity->findComponent(render::ComponentType::ParticleSystem));

    render::Entity* wallsContainer = getEntity()->parent->findNamedEntity("ScreenBoundary");
    m_wallLeftEntity = wallsContainer->findNamedEntity("Left");
    m_wallRightEntity = wallsContainer->findNamedEntity("Right");
    m_wallTopEntity = wallsContainer->findNamedEntity("Top");
    m_wallBottomEntity = wallsContainer->findNamedEntity("Bottom");

    m_powerupsContainerEntity = wallsContainer->findNamedEntity("PowerupsContainer");
    m_enemiesContainerEntity = wallsContainer->findNamedEntity("EnemiesContainer");
    
    // pinball stuff
    m_flipperLeftEntity = getEntity()->parent->findNamedEntity("FlipperLeft");
    m_flipperRightEntity = getEntity()->parent->findNamedEntity("FlipperRight");
    m_initialFlipperLeftRot = m_flipperLeftEntity->transform.getRotation();
    m_initialFlipperRightRot = m_flipperRightEntity->transform.getRotation();

    m_bumper = getEntity()->parent->findNamedEntity("Bumper");

    // for resetting the scene, for level transitions
    m_initialBallPos = m_ballEntity->transform.getPosition();
    m_initialPaddlePos = m_paddleEntity->transform.getPosition();

    // box2d setup
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = { 0, 0 };
    worldDef.maximumLinearVelocity = k_BALL_TERMINAL_VELOCITY * k_UNITS_TO_BOX2D_SCALE;
    m_world = b2CreateWorld(&worldDef);

    m_paddleBody = makePaddle(m_paddleEntity, { k_PADDLE_WIDTH, 1.5f });
    m_ballBody = makeBall(m_ballEntity, 1.0f);

    m_wallBodies[0] = makeWall(m_wallLeftEntity, m_wallLeftEntity->transform.getScale().xy);
    m_wallBodies[1] = makeWall(m_wallRightEntity, m_wallRightEntity->transform.getScale().xy);
    m_wallBodies[2] = makeWall(m_wallTopEntity, m_wallTopEntity->transform.getScale().xy);
    m_wallBodies[3] = makeWall(m_wallBottomEntity, m_wallBottomEntity->transform.getScale().xy);

    m_flipperLeftBody = makeFlipper(m_flipperLeftEntity, 0.1f, { 6.5f, 1.95057f }, false);
    m_flipperRightBody = makeFlipper(m_flipperRightEntity, 0.1f, { 6.5f, 1.95057f }, true);

    for (auto brickEntity : m_bricksEntityRoot->children) {
        b2BodyId brickPhysicsId = makeBrick(brickEntity.get(), { 3.0f, 1.5f }, m_bricksEntityRoot->transform.getPosition().xy);
        Brick* brickComponent = (Brick*)brickEntity->findComponent(render::ComponentType::UserBehaviour);
        brickComponent->setBrickId(brickPhysicsId);
    }

    m_ballPaddleJoint = makeWeldJoint(m_paddleBody, m_ballBody, {0, -1});

    LOG_INFO("Created Level Breakanoid::Game!");
    setLevel(0);
}

void LevelHandler::sleep() {

    // box2d cleanup
    b2DestroyWorld(m_world);
}

b2BodyId LevelHandler::box2dMakeBody(b2BodyType bodyType, render::Entity* entityData, bool fixedRotation, b2Vec2 posOffset, float angle) {
    ASSERT(entityData != nullptr);

    // creates a box2d body with given params
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = bodyType;
    bodyDef.position = b2Vec2({ entityData->transform.getPosition().x * k_UNITS_TO_BOX2D_SCALE, entityData->transform.getPosition().y * k_UNITS_TO_BOX2D_SCALE }) + posOffset * k_UNITS_TO_BOX2D_SCALE;
    bodyDef.rotation = b2MakeRot(angle * DEG2RAD);
    bodyDef.fixedRotation = fixedRotation;
    bodyDef.userData = entityData;

    return b2CreateBody(m_world, &bodyDef);
}

b2BodyId LevelHandler::makePaddle(render::Entity* entityData, hlslpp::float2 size) {

    b2BodyId paddleBody = box2dMakeBody(b2_dynamicBody, entityData);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.enableContactEvents = true;
    shapeDef.friction = 0.3f;
    shapeDef.restitution = 0.1f;

    shapeDef.filter.categoryBits = collisions::category::PADDLE;
    shapeDef.filter.maskBits = collisions::masks::PADDLE;
    
    b2Polygon boxCollider = b2MakeBox(size.x * k_UNITS_TO_BOX2D_SCALE * 0.5f, size.y * k_UNITS_TO_BOX2D_SCALE * 0.5f);
    b2CreatePolygonShape(paddleBody, &shapeDef, &boxCollider);

    return paddleBody;
}

b2BodyId LevelHandler::makeBall(render::Entity* entityData, float radius) {

    b2BodyId ballBody = box2dMakeBody(b2_dynamicBody, entityData);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.enableContactEvents = true;
    shapeDef.density = 1.0f;
    shapeDef.friction = 0.2f;
    shapeDef.restitution = 1.0f;

    shapeDef.filter.categoryBits = collisions::category::BALL;
    shapeDef.filter.maskBits = collisions::masks::BALL;

    b2Circle circleCollider = {
        .center = {0, 1},
        .radius = radius * k_UNITS_TO_BOX2D_SCALE
    };
    b2CreateCircleShape(ballBody, &shapeDef, &circleCollider);

    return ballBody;
}

b2BodyId LevelHandler::makeWall(render::Entity* entityData, hlslpp::float2 size, float angle) {

    b2BodyId wallBody = box2dMakeBody(b2_staticBody, entityData, true, b2Vec2_zero, angle);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.enableContactEvents = true;
    shapeDef.friction = 0.0f;

    shapeDef.filter.categoryBits = collisions::category::WALL;
    shapeDef.filter.maskBits = collisions::masks::WALL;

    b2Polygon boxCollider = b2MakeBox(size.x * k_UNITS_TO_BOX2D_SCALE * 0.5f, size.y * k_UNITS_TO_BOX2D_SCALE * 0.5f);
    b2CreatePolygonShape(wallBody, &shapeDef, &boxCollider);

    return wallBody;
}

b2BodyId LevelHandler::makeBrick(render::Entity* entityData, hlslpp::float2 size, hlslpp::float2 offset) {

    b2BodyId brickBody = box2dMakeBody(b2_staticBody, entityData, true, { offset.x, offset.y });

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.enableContactEvents = true;
    shapeDef.friction = 0.3f;

    shapeDef.filter.categoryBits = collisions::category::BRICK;
    shapeDef.filter.maskBits = collisions::masks::BRICK;

    b2Polygon boxCollider = b2MakeBox(size.x * k_UNITS_TO_BOX2D_SCALE * 0.5f, size.y * k_UNITS_TO_BOX2D_SCALE * 0.5f);
    b2CreatePolygonShape(brickBody, &shapeDef, &boxCollider);

    return brickBody;
}

b2BodyId LevelHandler::makePowerup(render::Entity* entityData, float radius) {

    b2BodyId powerupBody = box2dMakeBody(b2_dynamicBody, entityData);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.enableContactEvents = true;
    shapeDef.density = 1.0f;
    shapeDef.friction = 0.0f;
    shapeDef.restitution = 1.0f;

    shapeDef.filter.categoryBits = collisions::category::POWERUP;
    shapeDef.filter.maskBits = collisions::masks::POWERUP;

    b2Circle circleCollider = {
        .radius = radius * k_UNITS_TO_BOX2D_SCALE
    };
    b2CreateCircleShape(powerupBody, &shapeDef, &circleCollider);

    return powerupBody;
}

b2BodyId LevelHandler::makeFlipper(render::Entity* entityData, float pivotRadius, hlslpp::float2 flipperSize, bool flipX) {

    b2BodyId flipperPivot = box2dMakeBody(b2_staticBody, entityData, false, b2Vec2((flipX ? 0.52f : -0.25f) * flipperSize.x, 0), 0);
    b2BodyId flipperBody = box2dMakeBody(b2_dynamicBody, entityData, false);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.enableContactEvents = true;
    shapeDef.density = 1.0f;
    shapeDef.filter.categoryBits = collisions::category::FLIPPER;
    shapeDef.filter.maskBits = collisions::masks::FLIPPER;

    b2Polygon boxCollider = b2MakeBox(flipperSize.x * k_UNITS_TO_BOX2D_SCALE * 0.5f, flipperSize.y * k_UNITS_TO_BOX2D_SCALE * 0.5f);
    b2CreatePolygonShape(flipperBody, &shapeDef, &boxCollider);

    b2Circle circleCollider = {
        .center = {0, 0},
        .radius = pivotRadius * k_UNITS_TO_BOX2D_SCALE
    };
    shapeDef.filter.maskBits = 0; // ignore collisions on the pivot, we only have it for rotations
    b2CreateCircleShape(flipperPivot, &shapeDef, &circleCollider);

    b2RevoluteJointDef jointDef = b2DefaultRevoluteJointDef();
    jointDef.bodyIdA = flipperBody;
    jointDef.bodyIdB = flipperPivot;
    jointDef.enableLimit = true;
    jointDef.referenceAngle = 0 * DEG2RAD;
    jointDef.lowerAngle = k_FLIPPER_ANGLE_NEUTRAL * DEG2RAD;
    jointDef.upperAngle = k_FLIPPER_ANGLE_MAX * DEG2RAD;
    jointDef.localAnchorA = b2Vec2((flipX ? 0.5f : -0.5f) * flipperSize.x * k_UNITS_TO_BOX2D_SCALE, 0);
    jointDef.localAnchorB = b2Vec2(0, 0);
    b2JointId joint = b2CreateRevoluteJoint(m_world, &jointDef);

    return flipperBody;
}

b2JointId LevelHandler::makeWeldJoint(b2BodyId pA, b2BodyId pB, b2Vec2 anchorOffset) {
    b2WeldJointDef weldJointDef = b2DefaultWeldJointDef();
    weldJointDef.bodyIdA = pA;
    weldJointDef.bodyIdB = pB;
    b2Vec2 bodyAPos = b2Body_GetPosition(pA);
    b2Vec2 bodyBPos = b2Body_GetPosition(pB);
    weldJointDef.localAnchorA = bodyAPos - bodyBPos - (anchorOffset * k_UNITS_TO_BOX2D_SCALE);

    return b2CreateWeldJoint(m_world, &weldJointDef);
}
void LevelHandler::destroyWeldJoint(b2JointId& joint) {
    if (!B2_ID_EQUALS(joint, b2_nullJointId)) {
        b2DestroyJoint(joint);
        joint = b2_nullJointId;
    }
}

void LevelHandler::launchBall(float move) {
    if (!B2_ID_EQUALS(m_ballPaddleJoint, b2_nullJointId)) {
        destroyWeldJoint(m_ballPaddleJoint);

        // magnitude is between k_BALL_LAUNCH_VELOCITY_MIN and k_BALL_LAUNCH_VELOCITY_MAX.
        // factor is spaceHeldTime / k_MAX_SPACE_HELD_TIME_SECONDS clamped between 0 and 1
        float launchMagnitude = hlslpp::lerp(k_BALL_LAUNCH_VELOCITY_MIN, k_BALL_LAUNCH_VELOCITY_MAX, hlslpp::saturate((hlslpp::float1)m_spaceHeldTime / k_MAX_SPACE_HELD_TIME_SECONDS));
        
        // speed is paddle velocity + a vector up based on magnitude of the ball, this lets us fake momentum for launching the ball
        hlslpp::float2 speed = hlslpp::normalize(hlslpp::float2{ move * k_BALL_NORMAL_SPEED, k_PADDLE_VELOCITY * k_BALL_NORMAL_SPEED }) * launchMagnitude;
        b2Body_SetLinearVelocity(m_ballBody, { speed.x * k_LAUNCH_SPEED_SCALE, speed.y * k_LAUNCH_SPEED_SCALE });
        LOG_INFO("Launch mag: {} ({}, {}) mv = {}", launchMagnitude, (float)speed.x, (float)speed.y, move);
        m_currentBallSpeed = launchMagnitude;
    }
}

void LevelHandler::killBall() {
    m_lives--;
    if (m_lives >= 0) {
        // stop all movement on the ball and paddle
        b2Body_SetLinearVelocity(m_ballBody, { 0,0 });
        b2Body_SetLinearVelocity(m_paddleBody, { 0,0 });
        // reset transform
        b2Body_SetTransform(m_ballBody, { m_initialBallPos.x * k_UNITS_TO_BOX2D_SCALE, m_initialBallPos.y * k_UNITS_TO_BOX2D_SCALE }, b2Rot_identity);
        b2Body_SetTransform(m_paddleBody, { m_initialPaddlePos.x * k_UNITS_TO_BOX2D_SCALE, m_initialPaddlePos.y * k_UNITS_TO_BOX2D_SCALE }, b2Rot_identity);
        // stick the ball to the paddle
        m_ballPaddleJoint = makeWeldJoint(m_paddleBody, m_ballBody, { 0, -1 });
    } else {
        LOG_INFO("lol lmao skill issue dumbass bitch");
        // @TODO: Main menu
    }
}

void LevelHandler::activateFlipper(b2BodyId body, float& currentAngle, bool isFlipped) {
    if (!B2_ID_EQUALS(body, b2_nullJointId)) {
        currentAngle = 1.0f;
        // b2Body_ApplyForceToCenter(body, b2Vec2(0, 5000), true);
        b2Body_SetAngularVelocity(body, (isFlipped ? -1.0f : 1.0f) * k_FLIPPER_LAUNCH_VELOCITY);
    }
}

void LevelHandler::dampenFlipper(b2BodyId body, float& currentAngle, float deltaTime, bool isFlipped) {
    if (!B2_ID_EQUALS(body, b2_nullJointId)) {
        currentAngle = (1.0f - deltaTime / k_FLIPPER_KICK_DURATION_SECONDS);
        currentAngle = (hlslpp::float1) hlslpp::saturate((hlslpp::float1)currentAngle); // clamp 0-1
        b2Vec2 linVel = b2Body_GetLinearVelocity(body);
        float angularVelocity = b2Body_GetAngularVelocity(body);
        b2Body_SetAngularVelocity(body, angularVelocity + (isFlipped ? -1.0f : 1.0f) * k_FLIPPER_GRAVITY * (1.0f - currentAngle));
    }
}

void LevelHandler::update(float deltaTime) {
    // This is kinda hacky but i didnt have time to flesh out the physics portion of the ECS so do everything in a global manager in a big method :D

    // paddle movement
    hlslpp::float3 paddlePos = m_paddleEntity->transform.getPosition();
    float move = 0;
    if (engine::input::InputManager::getInstance()->keyDown(engine::input::Keycode::A) || engine::input::InputManager::getInstance()->keyDown(engine::input::Keycode::Left)) {
        move -= k_PADDLE_VELOCITY;
    }
    if (engine::input::InputManager::getInstance()->keyDown(engine::input::Keycode::D) || engine::input::InputManager::getInstance()->keyDown(engine::input::Keycode::Right)) {
        move += k_PADDLE_VELOCITY;
    }

    // powerups

    // pinball stuff
    if (engine::input::InputManager::getInstance()->mouseReleased(engine::input::MouseButton::ButtonLeft)) {
        activateFlipper(m_flipperLeftBody, m_flipperAngleLeft, false);
    }
    if (engine::input::InputManager::getInstance()->mouseReleased(engine::input::MouseButton::ButtonRight)) {
        activateFlipper(m_flipperRightBody, m_flipperAngleRight, true);
    }
    dampenFlipper(m_flipperLeftBody, m_flipperAngleLeft, deltaTime, false);
    dampenFlipper(m_flipperRightBody, m_flipperAngleRight, deltaTime, true);

    // we need to clamp the paddle so that it doesn't go out of bounds from the walls
    float nextPos = m_paddleEntity->transform.getPosition().x + move * deltaTime; // use deltatime here as we're trying to predict where the paddle would be in the next frame
    float wallLeftBoundary = (float)(m_wallLeftEntity->transform.getPosition().x + m_wallLeftEntity->transform.getScale().x * 0.5f);
    float wallRightBoundary = (float)(m_wallRightEntity->transform.getPosition().x - m_wallRightEntity->transform.getScale().x * 0.5f);
    if ((nextPos - k_PADDLE_WIDTH * 0.5f) < wallLeftBoundary) {
        move = 0;
    }
    if ((nextPos + k_PADDLE_WIDTH * 0.5f) > wallRightBoundary) {
        move = 0;
    }
    b2Body_SetLinearVelocity(m_paddleBody, { move * k_UNITS_TO_BOX2D_SCALE, 0 } );

    if (move != 0) {
        m_lastMove = move;
    }

    // ball update
    if (engine::input::InputManager::getInstance()->keyReleased(engine::input::Keycode::Space)) {
        launchBall(m_lastMove);
    }
    if (engine::input::InputManager::getInstance()->keyDown(engine::input::Keycode::Space)) {
        m_spaceHeldTime += deltaTime;
    } else {
        m_spaceHeldTime = 0;
    }

    // we need to update the ball's velocity here every frame so that it's constant and locked at 45 deg angles
    // we snap it to 45 deg angles to keep the game playable as its far too easy to get the ball stuck going left right / up down forever (also it's not fun like that)
    // only do this if the joint is absent (i.e. ball absent)
    if (B2_ID_EQUALS(m_ballPaddleJoint, b2_nullJointId)) {
        // @TODO: Maybe bad once we add the pinball shit??
        b2Vec2 ballVelocityCurr = b2Body_GetLinearVelocity(m_ballBody);
        hlslpp::float2 ballVelocityAdjusted = { ballVelocityCurr.x, ballVelocityCurr.y };
        ballVelocityAdjusted.x = ballVelocityAdjusted.x < 0 ? -1 : 1;
        ballVelocityAdjusted.y = ballVelocityAdjusted.y < 0 ? -1 : 1;
        ballVelocityAdjusted = hlslpp::normalize(ballVelocityAdjusted) * m_currentBallSpeed; // keep launch speed
        b2Body_SetLinearVelocity(m_ballBody, { ballVelocityAdjusted.x * k_UNITS_TO_BOX2D_SCALE, ballVelocityAdjusted.y * k_UNITS_TO_BOX2D_SCALE });
    }

    // attractors / repellers


    // simulate physics
    b2World_Step(m_world, deltaTime, physics::k_PHYSICS_SUBSTEP_COUNT);

    // Pop collision events, update game state
    b2ContactEvents events = b2World_GetContactEvents(m_world);

    for (int i = 0; i < events.beginCount; i++) {
        render::Entity* bodyA = ((render::Entity*)b2Body_GetUserData(b2Shape_GetBody(events.beginEvents[i].shapeIdA)));
        render::Entity* bodyB = ((render::Entity*)b2Body_GetUserData(b2Shape_GetBody(events.beginEvents[i].shapeIdB)));

        // check if the ball is one of the entities
        if (bodyA == m_ballEntity || bodyB == m_ballEntity) {
            // LOG_INFO("BIG BOUNCY BALLS");

            // Ok so we have the ball, now check if it went out of bounds
            if (bodyA == m_wallBottomEntity || bodyB == m_wallBottomEntity) {
                LOG_INFO("womp womp fuck you!");
                
                killBall();
            } else if (bodyA->parent == m_bricksEntityRoot || bodyB->parent == m_bricksEntityRoot) {

                LOG_INFO("*lego brick sound effect*");
                
                render::Entity* brick = nullptr;
                b2BodyId brickId = b2_nullBodyId;

                if (bodyA->parent == m_bricksEntityRoot) {
                    brick = bodyA;
                    brickId = b2Shape_GetBody(events.beginEvents[i].shapeIdA);
                } else if (bodyB->parent == m_bricksEntityRoot) {
                    brick = bodyB;
                    brickId = b2Shape_GetBody(events.beginEvents[i].shapeIdB);
                }

                // depending on brick type, award points or do something else
                render::Entity* brickEntity = (render::Entity*) b2Body_GetUserData(brickId);

                Brick* brickComponent = (Brick*)brickEntity->findComponent(render::ComponentType::UserBehaviour);
                brickComponent->onHit(); // Tell the brick that it got hit
                uint32_t pointsToAward =  brickComponent->getPoints();
                m_score += pointsToAward;

                if (brickComponent->isDestroyed()) {
                    m_bricksToProgressToNextLevel--;
                }

                if (brickComponent->shouldSpawnPowerup()) {
                    spawnPowerup();
                }

                if (!brickComponent->shouldExistInScene()) {
                    // disable bricks
                    brick->enabled = false;
                    b2Body_Disable(brickId);
                }
            }
        }
        // LOG_INFO("Begin {} with {}!", bodyA->name, bodyB->name);
    }

    // update positions before exiting
    paddlePos = m_paddleEntity->transform.getPosition();
    b2Vec2 paddlePosSim = b2Body_GetPosition(m_paddleBody);
    m_paddleEntity->transform.setPosition({ paddlePosSim.x * k_BOX2D_TO_UNITS_SCALE, paddlePos.y, paddlePos.z });

    hlslpp::float3 ballPos = m_ballEntity->transform.getPosition();
    b2Vec2 ballPosSim = b2Body_GetPosition(m_ballBody);
    m_ballEntity->transform.setPosition({ ballPosSim.x * k_BOX2D_TO_UNITS_SCALE, ballPosSim.y * k_BOX2D_TO_UNITS_SCALE, ballPos.z });

    // update flippers rotation
    float flipperLeftRotation = b2Rot_GetAngle(b2Body_GetRotation(m_flipperLeftBody));
    float flipperRightRotation = -b2Rot_GetAngle(b2Body_GetRotation(m_flipperRightBody)); // right is rotated 180 deg so we need to negate it to translate to the scene graph correctly
    m_flipperLeftEntity->transform.setRotation(hlslpp::mul(m_initialFlipperLeftRot, hlslpp::quaternion::rotation_euler_zxy({ 0 * DEG2RAD, 0 * DEG2RAD, flipperLeftRotation })));
    m_flipperRightEntity->transform.setRotation(hlslpp::mul(m_initialFlipperRightRot, hlslpp::quaternion::rotation_euler_zxy({ 0 * DEG2RAD, 0 * DEG2RAD, flipperRightRotation })));
    
    // PARTICLES!!!
    // ball trail
    if (m_ballParticleTimer > 0) {
        m_ballParticleTimer -= deltaTime;
    }
    if (B2_ID_EQUALS(m_ballPaddleJoint, b2_nullJointId) && m_ballParticleTimer <= 0) {
        hlslpp::float3 ballVelocityTrajectory = (m_ballEntity->transform.getPosition() - ballPos) * hlslpp::float3(5,5,5);
        m_ballParticleSystem->emit({
            .position = m_ballEntity->transform.getPosition(),
            .velocity = hlslpp::float3(0, 0, 0),
            .velocityVariation = ballVelocityTrajectory,
            .colourBegin = hlslpp::float4(1,1,1,1),
            .colourEnd = hlslpp::float4(1,1,1,0),
            .lifeTime = 5 /* seconds */,
        });
        m_ballParticleTimer = k_BALL_PARTICLE_FREQUENCY;
    }


    // graphics mode toggle
    if (engine::input::InputManager::getInstance()->keyReleased(engine::input::Keycode::F1)) {
        GraphicsModeTarget mode = GraphicsMode::getGraphicsMode();
        if (mode == GraphicsModeTarget::Classic) {
            GraphicsMode::setGraphicsMode(GraphicsModeTarget::Modern);
        }
        else {
            GraphicsMode::setGraphicsMode(GraphicsModeTarget::Classic);
            // in classic mode reset the camera to 0
            m_cameraEntity->transform.setRotation(hlslpp::quaternion::rotation_euler_zxy({ 0 * DEG2RAD, 0 * DEG2RAD, 0 * DEG2RAD }));
        }
    }

    // Camera should look at the ball when it's above a y = 2.5
    // only when in modern mode too
    if (GraphicsMode::getGraphicsMode() == GraphicsModeTarget::Modern && m_ballEntity->transform.getPosition().y > 2.5f) {
        // tiltFactor clamped between 0 and 1, clamping a bit before the top of the play area
        float tiltFactor = hlslpp::saturate((m_ballEntity->transform.getPosition().y - 2.5f) / (m_wallTopEntity->transform.getPosition().y - m_wallTopEntity->transform.getScale().y * 0.5f - 4.0f));
        m_cameraEntity->transform.setRotation(hlslpp::quaternion::rotation_euler_zxy({ easeInOutQuad(tiltFactor) * k_CAMERA_ANGLE_TILT_MAX * DEG2RAD, 0 * DEG2RAD, 0 * DEG2RAD }));
    }

    if (m_bricksToProgressToNextLevel == 0) {
        setLevel(m_level + 1);
    }
}

void LevelHandler::spawnPowerup() {
    render::Entity* newPowerUp = m_powerupsContainerEntity->push_back(
        render::EntityBuilder().withName("Powerup")
        .withPosition({})
        .withMeshRenderer({
            .mesh = engine::App::getInstance()->getAssetManager()->fetchMesh("smile.obj"),
            .material {
                .shader = m_shader,
                .name = "Powerup",
                .ambient = {0.5f, 0.5f, 0.5f},
                .diffuse = {1, 1, 1},
                .roughness = 1.0f,
                .diffuseTex = engine::App::getInstance()->getAssetManager()->fetchTexture("smile_albedo.png"),
                .matcapTex = engine::App::getInstance()->getAssetManager()->fetchTexture("hdri_matcap.png"),
                .brdfLutTex = engine::App::getInstance()->getAssetManager()->fetchTexture("dfg.hdr")
            }})
    );

    // @TODO: Other powerup shit
}

void LevelHandler::setLevelLayout(LevelBrickLayoutShape shape) {

    const int columns = 10;
    const int rows = 4;

    switch (shape) {
    case LevelBrickLayoutShape::Full:
    {
        LOG_INFO("Setting level layout to Full");
        for (auto brickEntity : m_bricksEntityRoot->children) {
            auto pBrickComponent = (Brick*)(brickEntity.get()->findComponent(render::ComponentType::UserBehaviour));

            pBrickComponent->getEntity()->enabled = true;
            b2Body_Enable(pBrickComponent->getBrickId());
        }
        break;
    }
    case LevelBrickLayoutShape::Diamond:
    {
        LOG_INFO("Setting level layout to Diamond");
        for (auto brickEntity : m_bricksEntityRoot->children) {
            auto pBrickComponent = (Brick*)(brickEntity.get()->findComponent(render::ComponentType::UserBehaviour));

            bool shouldEnable = true;
            if (pBrickComponent->getPosY() == 3 || pBrickComponent->getPosY() == 1) {
                shouldEnable = std::abs(pBrickComponent->getPosX() / 2 - 2) < 2;
            }
            if (pBrickComponent->getPosY() == 2) {
                shouldEnable = true;
            }
            if (pBrickComponent->getPosY() == 0) {
                shouldEnable = (pBrickComponent->getPosX() / 2 - 2) == 0;
            }

            if (shouldEnable) {
                pBrickComponent->getEntity()->enabled = true;
                b2Body_Enable(pBrickComponent->getBrickId());
            } else {
                pBrickComponent->getEntity()->enabled = false;
                b2Body_Disable(pBrickComponent->getBrickId());
            }
        }
        break;
    }
    case LevelBrickLayoutShape::Pyramid:
    {
        LOG_INFO("Setting level layout to Pyramid");
        for (auto brickEntity : m_bricksEntityRoot->children) {
            auto pBrickComponent = (Brick*)(brickEntity.get()->findComponent(render::ComponentType::UserBehaviour));

            int allowableDistance = pBrickComponent->getPosY();
            int distanceFromCenter = std::abs(pBrickComponent->getPosX() - (columns / 2));

            if (distanceFromCenter <= allowableDistance) {
                pBrickComponent->getEntity()->enabled = true;
                b2Body_Enable(pBrickComponent->getBrickId());
            } else {
                pBrickComponent->getEntity()->enabled = false;
                b2Body_Disable(pBrickComponent->getBrickId());
            }
        }
        break;
    }
    case LevelBrickLayoutShape::SemiCircle:
    {
        LOG_INFO("Setting level layout to Semi Circle");
        for (auto brickEntity : m_bricksEntityRoot->children) {
            auto pBrickComponent = (Brick*)(brickEntity.get()->findComponent(render::ComponentType::UserBehaviour));

            bool shouldEnable = true;
            if (pBrickComponent->getPosY() == 3) {
                shouldEnable = std::abs(pBrickComponent->getPosX() / 2) != 2;
            }
            if (pBrickComponent->getPosY() == 2) {
                shouldEnable = true;
            }
            if (pBrickComponent->getPosY() == 1) {
                shouldEnable = std::abs(pBrickComponent->getPosX() - 4.5f) < 4.0f;
            }
            if (pBrickComponent->getPosY() == 0) {
                shouldEnable = std::abs(pBrickComponent->getPosX() - 4.5f) < 2.0f;
            }

            if (shouldEnable) {
                pBrickComponent->getEntity()->enabled = true;
                b2Body_Enable(pBrickComponent->getBrickId());
            }
            else {
                pBrickComponent->getEntity()->enabled = false;
                b2Body_Disable(pBrickComponent->getBrickId());
            }
        }
        break;
    }
    case LevelBrickLayoutShape::Sparse:
    {
        LOG_INFO("Setting level layout to Sparse");
        for (auto brickEntity : m_bricksEntityRoot->children) {
            auto pBrickComponent = (Brick*)(brickEntity.get()->findComponent(render::ComponentType::UserBehaviour));

            if (pBrickComponent->getPosX() != 2 && pBrickComponent->getPosX() != 7) {
                pBrickComponent->getEntity()->enabled = true;
                b2Body_Enable(pBrickComponent->getBrickId());
            } else {
                pBrickComponent->getEntity()->enabled = false;
                b2Body_Disable(pBrickComponent->getBrickId());
            }
        }
        break;
    }
    }
}

void LevelHandler::setLevel(uint32_t levelId) {
    ASSERT(levelId < k_MAX_LEVELS);

    LOG_INFO("Setting level to level {}", levelId + 1);

    if (levelId == 0) {
        // reset lives if restarting
        m_lives = k_INITIAL_LIVES;
    }
    // keep current lives, kill ball, restore lives
    m_lives++;
    launchBall();
    killBall();

    // set the new level id to what we have stored internally
    m_level = std::min(levelId, 20U);

    LevelInitParams currentParams = s_levelInitParams[m_level];

    // enable / disable bricks according to layout pattern
    setLevelLayout(currentParams.shape);
    m_bricksToProgressToNextLevel = 0;

    uint32_t multihitsSpawned = 0;
    uint32_t indestructablesSpawned = 0;

    // shuffle bricks
    std::vector<std::shared_ptr<render::Entity>> shuffledBricks = m_bricksEntityRoot->children;
    std::shuffle(shuffledBricks.begin(), shuffledBricks.end(), engine::RandomNumberGenerator::getRng());

    for (auto brickEntity : shuffledBricks) {
        auto pBrickComponent = (Brick*)(brickEntity.get()->findComponent(render::ComponentType::UserBehaviour));
        if (!pBrickComponent->getEntity()->enabled)
            continue;

        // Determine brick type
        BrickType brickType = BrickType::Regular;

        if (indestructablesSpawned < currentParams.indestructableCount) {
            brickType = BrickType::Indestructable;
            indestructablesSpawned++;
        } else if (multihitsSpawned < currentParams.numMultihit) {
            brickType = BrickType::Strong;
            multihitsSpawned++;
        } else {
            brickType = BrickType::Regular;
        }

        pBrickComponent->updateBrick(brickType);

        if (brickType != BrickType::Indestructable) {
            m_bricksToProgressToNextLevel++;
        }
    }

    // reset camera rotation
    m_cameraEntity->transform.setRotation(hlslpp::quaternion::rotation_euler_zxy({ 0 * DEG2RAD, 0 * DEG2RAD, 0 * DEG2RAD }));
}

void LevelHandler::imgui() {

    // box2d cleanup
    ImGui::Begin("Level debug");
    ImGui::Text(fmt::format("Level {}", (m_level + 1)).c_str());
    ImGui::NewLine();
    ImGui::DragInt("Lives", &m_lives, 1, 0, 20);
    ImGui::Text(fmt::format("Score {}", m_score).c_str());
    ImGui::Text(fmt::format("Bricks For Next Level {}", m_bricksToProgressToNextLevel).c_str());
    ImGui::NewLine();

    ImGui::DragFloat("max camera angle", &k_CAMERA_ANGLE_TILT_MAX, 0.01f, 0, 15);
    ImGui::DragFloat("flipper angle neutral", &k_FLIPPER_ANGLE_NEUTRAL, 0.01f, 0, 15);
    ImGui::DragFloat("flipper angle max", &k_FLIPPER_ANGLE_MAX, 0.01f, 0, 15);
    ImGui::DragFloat("Ball Particle Frequency", &k_BALL_PARTICLE_FREQUENCY, 0.01f, 0, 15);
    ImGui::DragFloat("Launch speed", &k_LAUNCH_SPEED_SCALE, 1, 0, 50000);

    {
        // Debug UI for Graphics Mode
        GraphicsModeTarget mode = GraphicsMode::getGraphicsMode();
        const char* graphicsModeTargetTypes[] = { "Classic", "Modern" };
        auto tempGraphicsMode = mode;
        ImGui::ComboboxEx("Graphics Mode", (int*)&tempGraphicsMode, graphicsModeTargetTypes, IM_ARRAYSIZE(graphicsModeTargetTypes));
        if (mode != tempGraphicsMode) {
            GraphicsMode::setGraphicsMode(tempGraphicsMode);
        }
    }

    if (ImGui::Button("Kill ball")) {
        killBall();
    }

    if (ImGui::Button("Next level")) {
        setLevel(m_level + 1);
    }


    ImGui::End();
}