#include "ui_stuff.hpp"

#include "engine/input/input_manager.hpp"
#include "engine/core.hpp"
#include "engine/log.hpp"
#include "arkanoid/arkanoid_layer.hpp"

void MainMenuInteractions::start() {
    m_attachedUiComponent = (render::UIElement*) getEntity()->findComponent(render::ComponentType::UIElement);
}

void MainMenuInteractions::sleep() {

}

void MainMenuInteractions::update(float deltaTime) {

    if (m_attachedUiComponent->isMouseOver()) {
        // @TODO: Find better tint colour lol
        m_attachedUiComponent->textureTint = hlslpp::float4(1,0,1,1);

        if (engine::input::InputManager::getInstance()->mouseReleased(engine::input::MouseButton::Button0)) {
            // load game scene
            ((ArkanoidLayer*)m_layer)->setActiveScene(((ArkanoidLayer*)m_layer)->gameScene);
        }
    }
}