#pragma once

#include "engine/renderer/scene_graph.hpp"

namespace render {

    class UICanvas : public IComponent {
    public:
        UICanvas(Entity* parent) : IComponent(parent) {
            componentType = ::render::ComponentType::UICanvas;
        }
        ~UICanvas() = default;
    private:
        // derived classes are forbidden from modifying componentType
        using IComponent::componentType;
    };

    enum class UIElementType {
        Text,
        Sprite,
    };

    class UIElement : public IComponent {
    public:
        UIElement(Entity* parent) : IComponent(parent) {
            componentType = ::render::ComponentType::UIElement;
        }
        ~UIElement() = default;

        UIElementType uiType = UIElementType::Sprite;

        // dont use hlslpp::float2 because padding makes it use double the space
        float posX = 0;
        float posY = 0;
        float sizeX = 1;
        float sizeY = 1;
        float outlineWidth = 0;
        float textScale = 1.0f;

        gpu::ITexture* texture = nullptr;
        std::string text;
        hlslpp::float4 textColour = hlslpp::float4(1, 1, 1, 1);
        hlslpp::float4 outlineColour = hlslpp::float4(1, 1, 1, 0);

        hlslpp::float4 textureTint = hlslpp::float4(1, 1, 1, 1);

        // for ui interactons like mouse click etc
        bool isMouseOver();
    private:
        // derived classes are forbidden from modifying componentType
        using IComponent::componentType;
    };
}