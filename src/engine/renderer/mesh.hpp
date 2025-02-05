#pragma once

#include <inttypes.h>
#include <hlsl++.h>
#include <engine/gpu/idevice.hpp>
#include "engine/renderer/scene_graph.hpp"
#include "engine/renderer/material.hpp"

namespace render {

    // Commonly used vertex layouts
    struct PositionColorVertex {
        float position[3] = {};
        float color[3] = {};
        float uv[2] = {};
    };

    struct PositionNormalTexcoordVertex {
        float position[3] = {};
        float normal[3] = {};
        float uv[2] = {};
    };

    // Collection of mesh data
    struct Mesh {
    public:
        gpu::IBuffer* vertexBuffer = nullptr;
        gpu::IBuffer* indexBuffer = nullptr;
        gpu::IInputLayout* vertexLayout = nullptr; // WHY IS VAO TIED TO THE VERTEX BUFFER?????
        size_t triangleCount = 0;
    };

    class MeshRenderer : public IComponent {
    public:
        MeshRenderer(Entity* parent) : IComponent(parent) {
            componentType = ::render::ComponentType::MeshRenderer;
        }
        ~MeshRenderer() = default;

        Mesh mesh{};
        Material material;
    private:
        // derived classes are forbidden from modifying componentType
        using IComponent::componentType;
    };
}