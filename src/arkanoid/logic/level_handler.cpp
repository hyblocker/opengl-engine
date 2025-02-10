#include "level_handler.hpp"
#include "engine/core.hpp"
#include "engine/log.hpp"

#include "engine/input/input_manager.hpp"

#include <imgui.h>

// helpers to convert radians to degrees and back
#define DEG2RAD (3.14159265/180.0)
#define RAD2DEG (180/3.14159265)

constexpr float k_PADDLE_VELOCITY = 5000.0f;
constexpr float k_BALL_TERMINAL_SPEED = 10.0f;
constexpr float k_BALL_NORMAL_SPEED = 3500.0f;
// constexpr float k_BALL_LAUNCH_VELOCITY = 10000.0f;
constexpr float k_BALL_LAUNCH_VELOCITY = k_BALL_NORMAL_SPEED;

// conversion constants so that box2d works at 100u = 1m
constexpr float k_UNITS_TO_BOX2D_SCALE = 100.0f;
constexpr float k_BOX2D_TO_UNITS_SCALE = 0.01f;

constexpr uint32_t k_BRICKS_COLUMNS = 10;
constexpr uint32_t k_BRICKS_ROWS = 4;

void LevelHandler::start() {
    // get pointers to entities in the scene
    m_ballEntity = getEntity()->parent->findNamedEntity("Ball");
    m_paddleEntity = getEntity()->parent->findNamedEntity("Paddle");
    m_cameraEntity = getEntity()->parent->findNamedEntity("Camera");
    m_bricksEntityRoot = getEntity()->parent->findNamedEntity("BrickContainer");

    render::Entity* wallsContainer = getEntity()->parent->findNamedEntity("ScreenBoundary");
    m_wallLeftEntity = wallsContainer->findNamedEntity("Left");
    m_wallRightEntity = wallsContainer->findNamedEntity("Right");
    m_wallTopEntity = wallsContainer->findNamedEntity("Top");
    m_wallBottomEntity = wallsContainer->findNamedEntity("Bottom");

    // for resetting the scene, for level transitions
    initialBallPos = m_ballEntity->transform.getPosition();
    initialPaddlePos = m_paddleEntity->transform.getPosition();

    // box2d setup
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = { 0, 0 };
    worldDef.maximumLinearVelocity = 100000.0f * k_UNITS_TO_BOX2D_SCALE;
    m_world = b2CreateWorld(&worldDef);

    m_paddleBody = makePaddle(m_paddleEntity, { 10, 1.5f });
    m_ballBody = makeBall(m_ballEntity, 1.0f);

    m_wallBodies[0] = makeWall(m_wallLeftEntity, m_wallLeftEntity->transform.getScale().xy);
    m_wallBodies[1] = makeWall(m_wallRightEntity, m_wallRightEntity->transform.getScale().xy);
    m_wallBodies[2] = makeWall(m_wallTopEntity, m_wallTopEntity->transform.getScale().xy);
    m_wallBodies[3] = makeWall(m_wallBottomEntity, m_wallBottomEntity->transform.getScale().xy);

    for (auto brickEntity : m_bricksEntityRoot->children) {
        makeBrick(brickEntity.get(), { 3.0f, 1.5f }, m_bricksEntityRoot->transform.getPosition().xy);
    }

    m_ballPaddleJoint = makeWeldJoint(m_paddleBody, m_ballBody, {0, -1});

    LOG_INFO("Created Level Breakanoid::Game!");
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
    bodyDef.fixedRotation = true;
    bodyDef.userData = entityData;

    return b2CreateBody(m_world, &bodyDef);
}

b2BodyId LevelHandler::makePaddle(render::Entity* entityData, hlslpp::float2 size) {

    b2BodyId paddleBody = box2dMakeBody(b2_kinematicBody, entityData);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.enableContactEvents = true;
    shapeDef.friction = 0.3f;

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
        .center = {0, 0},
        .radius = radius * k_UNITS_TO_BOX2D_SCALE
    };
    b2CreateCircleShape(ballBody, &shapeDef, &circleCollider);

    return ballBody;
}

b2BodyId LevelHandler::makeWall(render::Entity* entityData, hlslpp::float2 size, float angle) {

    b2BodyId wallBody = box2dMakeBody(b2_staticBody, entityData, true, b2Vec2_zero, angle);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.enableContactEvents = true;
    shapeDef.friction = 0.3f;

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
    shapeDef.friction = 0.3f;

    shapeDef.filter.categoryBits = collisions::category::POWERUP;
    shapeDef.filter.maskBits = collisions::masks::POWERUP;

    b2Circle circleCollider = {
        .radius = radius * k_UNITS_TO_BOX2D_SCALE
    };
    b2CreateCircleShape(powerupBody, &shapeDef, &circleCollider);

    return powerupBody;
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

        // speed is paddle velocity + a vector up based on magnitude of the ball, this lets us fake momentum for launching the ball
        hlslpp::float2 speed = hlslpp::normalize(hlslpp::float2{ move * k_BALL_NORMAL_SPEED, k_BALL_NORMAL_SPEED }) * k_BALL_LAUNCH_VELOCITY;
        b2Body_SetLinearVelocity(m_ballBody, { speed.x, speed.y });
    }
}

void LevelHandler::killBall() {
    m_lives--;
    // if (m_lives >= 0) {
        // stop all movement on the ball and paddle
        b2Body_SetLinearVelocity(m_ballBody, { 0,0 });
        b2Body_SetLinearVelocity(m_paddleBody, { 0,0 });
        // reset transform
        b2Body_SetTransform(m_ballBody, { initialBallPos.x * k_UNITS_TO_BOX2D_SCALE, initialBallPos.y * k_UNITS_TO_BOX2D_SCALE }, b2Rot_identity);
        b2Body_SetTransform(m_paddleBody, { initialPaddlePos.x * k_UNITS_TO_BOX2D_SCALE, initialPaddlePos.y * k_UNITS_TO_BOX2D_SCALE }, b2Rot_identity);
        // stick the ball to the paddle
        m_ballPaddleJoint = makeWeldJoint(m_paddleBody, m_ballBody, { 0, -1 });
    // } else {
        LOG_INFO("lol lmao skill issue dumbass bitch");
        // @TODO: Main menu
    // }
}

void LevelHandler::update(float deltaTime) {

    // This is kinda hacky but i didnt have time to flesh out the physics portion of the ECS so do everything in a global manager in a big method :D

    // paddle movement
    float move = 0;
    if (engine::input::InputManager::getInstance()->keyDown(engine::input::Keycode::A) || engine::input::InputManager::getInstance()->keyDown(engine::input::Keycode::Left)) {
        move -= k_PADDLE_VELOCITY;
    }
    if (engine::input::InputManager::getInstance()->keyDown(engine::input::Keycode::D) || engine::input::InputManager::getInstance()->keyDown(engine::input::Keycode::Right)) {
        move += k_PADDLE_VELOCITY;
    }

    // powerups

    // pinball stuff

    // @TODO: Box2d that shit
    b2Body_SetLinearVelocity(m_paddleBody, { move * k_UNITS_TO_BOX2D_SCALE * deltaTime, 0 } );


    // ball update
    if (engine::input::InputManager::getInstance()->keyReleased(engine::input::Keycode::Space)) {
        launchBall(move);
    }
    // we need to update the ball's velocity here every frame so that it's constant and locked at 45 deg angles
    // we snap it to 45 deg angles to keep the game playable as its far too easy to get the ball stuck going left right / up down forever (also it's not fun like that)
    // only do this if the joint is absent (i.e. ball absent)
    if (B2_ID_EQUALS(m_ballPaddleJoint, b2_nullJointId)) {
        b2Vec2 ballVelocityCurr = b2Body_GetLinearVelocity(m_ballBody);
        hlslpp::float2 ballVelocityAdjusted = { ballVelocityCurr.x, ballVelocityCurr.y };
        ballVelocityAdjusted.x = ballVelocityAdjusted.x < 0 ? -1 : 1;
        ballVelocityAdjusted.y = ballVelocityAdjusted.y < 0 ? -1 : 1;
        ballVelocityAdjusted = hlslpp::normalize(ballVelocityAdjusted) * k_BALL_NORMAL_SPEED;
        b2Body_SetLinearVelocity(m_ballBody, { ballVelocityAdjusted.x * k_UNITS_TO_BOX2D_SCALE * deltaTime, ballVelocityAdjusted.y * k_UNITS_TO_BOX2D_SCALE * deltaTime });
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
            }
            else if (bodyA->parent == m_bricksEntityRoot || bodyB->parent == m_bricksEntityRoot) {

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

                // disable bricks
                brick->enabled = false;
                b2Body_Disable(brickId);
                // depending on brick type, generate points

            }
        }
        // LOG_INFO("Begin {} with {}!", bodyA->name, bodyB->name);
    }

    // update positions before exiting
    hlslpp::float3 paddlePos = m_paddleEntity->transform.getPosition();
    b2Vec2 paddlePosSim = b2Body_GetPosition(m_paddleBody);
    m_paddleEntity->transform.setPosition({ paddlePosSim.x * k_BOX2D_TO_UNITS_SCALE, paddlePos.y, paddlePos.z });

    hlslpp::float3 ballPos = m_ballEntity->transform.getPosition();
    b2Vec2 ballPosSim = b2Body_GetPosition(m_ballBody);
    m_ballEntity->transform.setPosition({ ballPosSim.x * k_BOX2D_TO_UNITS_SCALE, ballPosSim.y * k_BOX2D_TO_UNITS_SCALE, ballPos.z });
}

void LevelHandler::imgui() {

    // box2d cleanup
    ImGui::Begin("Level debug");
    ImGui::Text(fmt::format("Lives {}", m_lives).c_str());
    ImGui::End();
}