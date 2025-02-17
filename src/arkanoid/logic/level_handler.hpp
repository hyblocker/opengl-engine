#pragma once

#include "engine/renderer/scene_graph.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/renderer/particle_system.hpp"
#include "engine/renderer/ui_components.hpp"
#include "leaderboard.hpp"

// we use collision matrices to mask out certain types of entities from colliding with one another
namespace collisions {
    namespace category {
        constexpr uint64_t BALL     = 1u << 0;
        constexpr uint64_t PADDLE   = 1u << 1;
        constexpr uint64_t WALL     = 1u << 2;
        constexpr uint64_t BRICK    = 1u << 3;
        constexpr uint64_t POWERUP  = 1u << 4;
        constexpr uint64_t FLIPPER  = 1u << 5;
        constexpr uint64_t BUMPER   = 1u << 6;
        constexpr uint64_t ENEMY    = 1u << 7;
        constexpr uint64_t LASER    = 1u << 8;
    }
    namespace masks {
        constexpr uint64_t BALL     = category::WALL | category::PADDLE | category::BRICK | category::FLIPPER | category::BUMPER | category::ENEMY;
        constexpr uint64_t PADDLE   = category::BALL | category::WALL | category::POWERUP;
        constexpr uint64_t WALL     = category::BALL | category::POWERUP | category::FLIPPER;
        constexpr uint64_t BRICK    = category::BALL;
        constexpr uint64_t POWERUP  = category::PADDLE | category::WALL;
        constexpr uint64_t FLIPPER  = category::BALL | category::WALL | category::POWERUP;
        constexpr uint64_t BUMPER   = category::BUMPER | category::BALL;
        constexpr uint64_t ENEMY    = category::WALL | category::BALL | category::PADDLE;
        constexpr uint64_t LASER    = category::ENEMY | category::BRICK;
    }
}

constexpr int32_t k_INITIAL_LIVES = 3;

enum class GameState {
    Gameplay,
    GameOver,
    Victory,
};

enum class PowerupType {
    PaddleExpansion,
    MultiBall,
    SlowMotion,
    StickyPaddle,
    LaserCannon,
    Count,
};

class LevelHandler : public render::IBehaviour {
public:
    LevelHandler(render::Entity* parent, gpu::IShader* classicShader) : IBehaviour(parent) { m_classicShader = classicShader; }
    ~LevelHandler() = default;

    void start() override;
    void sleep() override;
    void update(float deltaTime) override;
    void imgui() override;

    // setups the scene for a particular level layout. effectively a "load level"
    void setLevel(uint32_t levelId);
    void restart();

    inline const b2WorldId getWorldId() const { return m_world; }

    bool firstFrame = false;

private:
    struct FlipperPhysics {
        b2BodyId pivot = b2_nullBodyId;
        b2BodyId body = b2_nullBodyId;
        b2JointId joint = b2_nullJointId;
    };

    b2BodyId box2dMakeBody(b2BodyType bodyType, render::Entity* entityData, bool fixedRotation = true, b2Vec2 posOffset = b2Vec2_zero, float angle = 0);

    b2BodyId makePaddle(render::Entity* entityData, hlslpp::float2 size);
    b2BodyId makeBall(render::Entity* entityDat, float radius);
    b2BodyId makeWall(render::Entity* entityData, hlslpp::float2 size, float angle = 0);
    b2BodyId makeBrick(render::Entity* entityData, hlslpp::float2 size, hlslpp::float2 offset);
    b2BodyId makePowerup(render::Entity* entityData, float radius);
    FlipperPhysics makeFlipper(render::Entity* entityData, float pivotRadius, hlslpp::float2 flipperSize, bool flipX);
    b2BodyId makeBumper(render::Entity* entityData, float radius);
    b2BodyId makeEnemy(render::Entity* entityData, float radius);

    void spawnPowerup(hlslpp::float3 brickPos);

    b2JointId makeWeldJoint(b2BodyId pA, b2BodyId pB, b2Vec2 anchorOffset = b2Vec2_zero);
    void destroyWeldJoint(b2JointId& joint);
    
    void launchBall(float move = 0);
    void killBall();

    void killPowerup(render::Entity* powerup, b2BodyId powerupBody);
    void equipRandomPowerup();

    void expandPaddle();
    void shrinkPaddle();

    void activateFlipper(b2BodyId body, float& currentAngle, bool isFlipped);
    void dampenFlipper(b2BodyId body, float& currentAngle, float deltaTime, bool isFlipped);

    enum LevelBrickLayoutShape;
    void setLevelLayout(LevelBrickLayoutShape shape);

    void setGameOver();
    void setWin();

private:

    render::Entity* m_paddleEntity = nullptr;
    render::Entity* m_ballEntity = nullptr;
    render::Entity* m_bricksEntityRoot = nullptr;
    render::Entity* m_cameraEntity = nullptr;

    // walls
    render::Entity* m_wallTopEntity = nullptr;
    render::Entity* m_wallBottomEntity = nullptr;
    render::Entity* m_wallRightEntity = nullptr;
    render::Entity* m_wallLeftEntity = nullptr;

    // containers
    render::Entity* m_powerupsContainerEntity = nullptr;
    render::Entity* m_enemiesContainerEntity = nullptr;

    // pinball stuff
    render::Entity* m_rampLeft = nullptr;
    render::Entity* m_rampRight = nullptr;

    render::Entity* m_bumperEntity = nullptr;

    render::Entity* m_flipperLeftEntity = nullptr;
    render::Entity* m_flipperRightEntity = nullptr;

    // particle systems
    render::ParticleSystem* m_ballParticleSystem = nullptr;
    render::ParticleSystem* m_brickParticleSystem = nullptr;
    float m_ballParticleTimer = 0;

    // For making powerups and enemies
    gpu::IShader* m_shader = nullptr;
    gpu::IShader* m_classicShader = nullptr;

    // box2d props
    b2WorldId m_world = b2_nullWorldId;
    b2BodyId m_paddleBody = b2_nullBodyId;
    b2BodyId m_paddleBodyExpanded = b2_nullBodyId;
    b2BodyId m_ballBody = b2_nullBodyId;
    b2BodyId m_wallBodies[4] = { b2_nullBodyId, b2_nullBodyId, b2_nullBodyId, b2_nullBodyId };
    
    b2JointId m_ballPaddleJoint = b2_nullJointId;

    FlipperPhysics m_flipperLeftBody;
    FlipperPhysics m_flipperRightBody;

    b2BodyId m_bumperBody = b2_nullBodyId;

    std::vector<b2BodyId> m_powerupsPhysics;
    std::vector<b2BodyId> m_enemiesPhysics;

    struct PowerupData {
        PowerupType type = PowerupType::Count;
        float elapsedTime = 0;
    };

    std::vector<PowerupData> m_activePowerups;

    hlslpp::float3 m_initialBallPos;
    hlslpp::float3 m_initialPaddlePos;

    hlslpp::quaternion m_initialFlipperLeftRot;
    hlslpp::quaternion m_initialFlipperRightRot;

    float m_flipperAngleLeft = 0;
    float m_flipperAngleRight = 0;

    int32_t m_lives = k_INITIAL_LIVES;
    int32_t m_score = 0;
    int32_t m_oldScore = 0;
    int32_t m_bricksToProgressToNextLevel = 10*4;
    uint32_t m_level = 0;

    float m_currentBallSpeed = 0;
    float m_normalBallSpeed = 0;
    float m_paddleWidthFactor = 1.0f;
    float m_spaceHeldTime = 0;
    float m_lastMove = 0;
    bool m_isPaddleExpanded = false;

    render::UIElement* m_livesUi = nullptr;
    render::UIElement* m_levelsUi = nullptr;
    render::UIElement* m_scoresUi = nullptr;
    render::UIElement* m_gameoverUsernameInput = nullptr;
    render::UIElement* m_gameoverUsernameTooltip = nullptr;
    render::UIElement* m_gameoverLeaderboard = nullptr;
    render::UIElement* m_victoryUsernameInput = nullptr;
    render::UIElement* m_victoryUsernameTooltip = nullptr;
    render::UIElement* m_victoryLeaderboard = nullptr;

    GameState m_gameState = GameState::Gameplay;

    render::Entity* m_gameoverUiRoot = nullptr;
    render::Entity* m_victoryUiRoot = nullptr;

    char m_userNameBuffer[8] = {};
    uint32_t m_usernameBufferPointer = 0;
    bool m_isUsernameAccepted = false;

    std::string m_leaderboardFilePath;
    Leaderboard m_leaderboard;

    enum LevelBrickLayoutShape {
        Full,
        Pyramid,
        Diamond,
        SemiCircle,
        Sparse,
    };

    struct LevelInitParams {
        LevelBrickLayoutShape shape = LevelBrickLayoutShape::Full;
        uint32_t indestructableCount = 0;
        uint32_t numMultihit = 0;
        uint32_t easyEnemyCount = 0;
        uint32_t difficultEnemyCount = 0;
        bool enablePinballFlippers = false;
        bool enableRamps = false;
        bool enableBumpers = false;
        bool enableAttractors = false;
        bool enableRepellers = false;
    };

    const LevelInitParams s_levelInitParams[20] = {
        { .shape = LevelBrickLayoutShape::Full,        .indestructableCount = 0, .numMultihit = 0,  .easyEnemyCount = 0, .difficultEnemyCount = 0, .enablePinballFlippers = false, .enableRamps = false, .enableBumpers = false, .enableAttractors = false, .enableRepellers = false },
        { .shape = LevelBrickLayoutShape::Diamond,     .indestructableCount = 0, .numMultihit = 5,  .easyEnemyCount = 0, .difficultEnemyCount = 0, .enablePinballFlippers = false, .enableRamps = false, .enableBumpers = false, .enableAttractors = false, .enableRepellers = false },
        { .shape = LevelBrickLayoutShape::Pyramid,     .indestructableCount = 4, .numMultihit = 5,  .easyEnemyCount = 0, .difficultEnemyCount = 0, .enablePinballFlippers = true, .enableRamps = false, .enableBumpers = false, .enableAttractors = false, .enableRepellers = false },
        { .shape = LevelBrickLayoutShape::SemiCircle,  .indestructableCount = 4, .numMultihit = 10, .easyEnemyCount = 0, .difficultEnemyCount = 0, .enablePinballFlippers = false, .enableRamps = false, .enableBumpers = false, .enableAttractors = false, .enableRepellers = false },
        { .shape = LevelBrickLayoutShape::Sparse,      .indestructableCount = 4, .numMultihit = 15, .easyEnemyCount = 0, .difficultEnemyCount = 0, .enablePinballFlippers = true, .enableRamps = false, .enableBumpers = false, .enableAttractors = false, .enableRepellers = false },

        { .shape = LevelBrickLayoutShape::Full,        .indestructableCount = 4, .numMultihit = 4, .easyEnemyCount = 0, .difficultEnemyCount = 0, .enablePinballFlippers = false, .enableRamps = false, .enableBumpers = true, .enableAttractors = false, .enableRepellers = false },
        { .shape = LevelBrickLayoutShape::Pyramid,     .indestructableCount = 4, .numMultihit = 8, .easyEnemyCount = 0, .difficultEnemyCount = 0, .enablePinballFlippers = true, .enableRamps = false, .enableBumpers = true, .enableAttractors = false, .enableRepellers = false },
        { .shape = LevelBrickLayoutShape::Diamond,     .indestructableCount = 4, .numMultihit = 8, .easyEnemyCount = 0, .difficultEnemyCount = 0, .enablePinballFlippers = true, .enableRamps = true, .enableBumpers = true, .enableAttractors = false, .enableRepellers = false },
        { .shape = LevelBrickLayoutShape::SemiCircle,  .indestructableCount = 4, .numMultihit = 8, .easyEnemyCount = 0, .difficultEnemyCount = 0, .enablePinballFlippers = true, .enableRamps = true, .enableBumpers = true, .enableAttractors = false, .enableRepellers = true },
        { .shape = LevelBrickLayoutShape::Sparse,      .indestructableCount = 4, .numMultihit = 8, .easyEnemyCount = 0, .difficultEnemyCount = 0, .enablePinballFlippers = true, .enableRamps = true, .enableBumpers = true, .enableAttractors = true, .enableRepellers = true },

        { .shape = LevelBrickLayoutShape::Diamond,     .indestructableCount = 5, .numMultihit = 8, .easyEnemyCount = 1, .difficultEnemyCount = 0, .enablePinballFlippers = false, .enableRamps = false, .enableBumpers = true, .enableAttractors = true, .enableRepellers = false },
        { .shape = LevelBrickLayoutShape::Full,        .indestructableCount = 5, .numMultihit = 8, .easyEnemyCount = 2, .difficultEnemyCount = 0, .enablePinballFlippers = true, .enableRamps = true, .enableBumpers = false, .enableAttractors = true, .enableRepellers = true },
        { .shape = LevelBrickLayoutShape::Sparse,      .indestructableCount = 5, .numMultihit = 8, .easyEnemyCount = 2, .difficultEnemyCount = 0, .enablePinballFlippers = true, .enableRamps = true, .enableBumpers = true, .enableAttractors = false, .enableRepellers = false },
        { .shape = LevelBrickLayoutShape::SemiCircle,  .indestructableCount = 5, .numMultihit = 8, .easyEnemyCount = 2, .difficultEnemyCount = 0, .enablePinballFlippers = false, .enableRamps = false, .enableBumpers = true, .enableAttractors = false, .enableRepellers = true },
        { .shape = LevelBrickLayoutShape::Pyramid,     .indestructableCount = 5, .numMultihit = 8, .easyEnemyCount = 4, .difficultEnemyCount = 0, .enablePinballFlippers = true, .enableRamps = true, .enableBumpers = false, .enableAttractors = true, .enableRepellers = true },

        { .shape = LevelBrickLayoutShape::Diamond,     .indestructableCount = 6, .numMultihit = 8, .easyEnemyCount = 1, .difficultEnemyCount = 1, .enablePinballFlippers = true, .enableRamps = true, .enableBumpers = true, .enableAttractors = true, .enableRepellers = false },
        { .shape = LevelBrickLayoutShape::Pyramid,     .indestructableCount = 6, .numMultihit = 8, .easyEnemyCount = 4, .difficultEnemyCount = 1, .enablePinballFlippers = true, .enableRamps = true, .enableBumpers = true, .enableAttractors = false, .enableRepellers = true },
        { .shape = LevelBrickLayoutShape::SemiCircle,  .indestructableCount = 6, .numMultihit = 8, .easyEnemyCount = 4, .difficultEnemyCount = 2, .enablePinballFlippers = true, .enableRamps = true, .enableBumpers = false, .enableAttractors = true, .enableRepellers = true },
        { .shape = LevelBrickLayoutShape::Full,        .indestructableCount = 6, .numMultihit = 8, .easyEnemyCount = 4, .difficultEnemyCount = 3, .enablePinballFlippers = true, .enableRamps = true, .enableBumpers = true, .enableAttractors = true, .enableRepellers = true },
        { .shape = LevelBrickLayoutShape::Sparse,      .indestructableCount = 6, .numMultihit = 8, .easyEnemyCount = 4, .difficultEnemyCount = 4, .enablePinballFlippers = true, .enableRamps = true, .enableBumpers = true, .enableAttractors = true, .enableRepellers = true },
    };
};