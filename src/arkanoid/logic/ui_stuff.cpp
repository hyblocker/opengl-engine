#include "ui_stuff.hpp"

#include "engine/input/input_manager.hpp"
#include "engine/core.hpp"
#include "engine/log.hpp"
#include "arkanoid/arkanoid_layer.hpp"

#include "level_handler.hpp"

const hlslpp::float4 k_TINT_COLOUR = hlslpp::float4(0.57490196078f, 1, 1, 1);
const hlslpp::float4 k_NORMAL_COLOUR = hlslpp::float4(1, 1, 1, 1);

void MainMenuInteractions::start() {
    m_attachedUiComponent = (render::UIElement*) getEntity()->findComponent(render::ComponentType::UIElement);
    
}

void MainMenuInteractions::sleep() {

}

void MainMenuInteractions::update(float deltaTime) {

    if (m_attachedUiComponent->isMouseOver()) {
        m_attachedUiComponent->textureTint = k_TINT_COLOUR;

        if (engine::input::InputManager::getInstance()->mouseReleased(engine::input::MouseButton::Button0)) {
            // load game scene
            ((ArkanoidLayer*)m_layer)->setActiveScene(((ArkanoidLayer*)m_layer)->gameScene);
        }
    } else {
        m_attachedUiComponent->textureTint = k_NORMAL_COLOUR;
    }
}

void GameplayUiInteractions::start() {
    m_attachedUiComponent = (render::UIElement*)getEntity()->findComponent(render::ComponentType::UIElement);
    m_levelHandler = (LevelHandler*) (((ArkanoidLayer*)m_layer)->gameScene.findNamedEntity("GameManager")->findComponent(render::ComponentType::UserBehaviour));
}

void GameplayUiInteractions::sleep() {

}

void GameplayUiInteractions::update(float deltaTime) {

    if (m_attachedUiComponent->isMouseOver()) {
        m_attachedUiComponent->textureTint = k_TINT_COLOUR;

        if (engine::input::InputManager::getInstance()->mouseReleased(engine::input::MouseButton::Button0)) {
            switch (m_gameplayType) {
            case GameplayButtonClass::Restart: {
                m_levelHandler->restart();
                break;
            }
            case GameplayButtonClass::ReturnToMenu: {
                // load menu scene
                m_levelHandler->restart();
                ((ArkanoidLayer*)m_layer)->setActiveScene(((ArkanoidLayer*)m_layer)->menuScene);
                break;
            }
            }
        }
    } else {
        m_attachedUiComponent->textureTint = k_NORMAL_COLOUR;
    }
}