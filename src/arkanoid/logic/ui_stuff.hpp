#pragma once

#include "engine/renderer/scene_graph.hpp"
#include "engine/renderer/ui_components.hpp"
#include "engine/ilayer.hpp"

class MainMenuInteractions : public render::IBehaviour {
public:
    MainMenuInteractions(render::Entity* parent, engine::ILayer* layer) : IBehaviour(parent) { m_layer = layer; }
    ~MainMenuInteractions() = default;

    void start() override;
    void sleep() override;
    void update(float deltaTime) override;

private:
    engine::ILayer* m_layer = nullptr;
    render::UIElement* m_attachedUiComponent = nullptr;
};

enum GameplayButtonClass {
    Restart,
    ReturnToMenu,
};

class LevelHandler;

class GameplayUiInteractions : public render::IBehaviour {
public:
    GameplayUiInteractions(render::Entity* parent, engine::ILayer* layer, GameplayButtonClass gameplayType)
        : IBehaviour(parent) {
        m_layer = layer;
        m_gameplayType = gameplayType;
    }

    ~GameplayUiInteractions() = default;

    void start() override;
    void sleep() override;
    void update(float deltaTime) override;

private:
    engine::ILayer* m_layer = nullptr;
    render::UIElement* m_attachedUiComponent = nullptr;

    GameplayButtonClass m_gameplayType = GameplayButtonClass::Restart;
    LevelHandler* m_levelHandler = nullptr;
};