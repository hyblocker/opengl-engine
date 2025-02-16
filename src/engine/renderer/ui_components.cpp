#include "ui_components.hpp"

#include "engine/input/input_manager.hpp"
#include "engine/app.hpp"

namespace render {
    bool UIElement::isMouseOver() {

        hlslpp::float2 mousePos = engine::input::InputManager::getInstance()->mousePos();

        hlslpp::float2 windowSize = { engine::App::getInstance()->getWindow()->getWidth(), engine::App::getInstance()->getWindow()->getHeight() };

        hlslpp::float2 position = { posX, posY };
        hlslpp::float2 size = { sizeX, sizeY };
        hlslpp::float2 topLeftCoord = windowSize * 0.5f + position - size * 0.5f;
        hlslpp::float2 bottomRightCoord = topLeftCoord + size;

        return topLeftCoord.x < mousePos.x && mousePos.x < bottomRightCoord.x &&
            topLeftCoord.y < mousePos.y && mousePos.y < bottomRightCoord.y;
    }
}