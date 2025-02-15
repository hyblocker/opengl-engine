#pragma once

#include <inttypes.h>
#include <hlsl++.h>
#include <engine/gpu/idevice.hpp>
#include "engine/renderer/scene_graph.hpp"
#include "engine/renderer/material.hpp"

#include <vector>

namespace render {

    class SceneRenderer;

    class ParticleSystem : public IComponent {

        friend class SceneRenderer;

    public:

        struct ParticleParams {
            hlslpp::float3 position = {0,0,0};
            hlslpp::float3 velocity = { 0,0,0 }, velocityVariation = {1,1,1};
            hlslpp::float4 colourBegin = { 1,1,1,1 }, colourEnd = {1,1,1,0};
            float sizeBegin = 1, sizeEnd = 0, sizeVariation = 0.1f;
            float lifeTime = 1;
        };

        ParticleSystem(Entity* parent) : IComponent(parent) {
            componentType = ::render::ComponentType::ParticleSystem;
            m_particlePool.resize(800);
            clear();
        }
        ~ParticleSystem() = default;

        void start();
        void emit(ParticleParams params);
        void update(float deltaTime);
        void clear();

        inline uint32_t getActiveParticleCount() const { return m_particleCount; }

        Material material = {};
        gpu::IBlendState* blendState = nullptr;

        uint32_t particleTextureCount = 1;

    private:
        // derived classes are forbidden from modifying componentType
        using IComponent::componentType;

        struct ParticleInstance {
            hlslpp::float3 position;
            hlslpp::float3 velocity;
            hlslpp::float4 colourBegin, colourEnd;
            float sizeBegin, sizeEnd;
            float lifeTime = 1;
            float lifeRemaining = 0;
            bool alive = false;
        };

        // basically a ring buffer
        std::vector<ParticleInstance> m_particlePool;
        uint32_t m_poolIndex = 799; // hard limit because of UBO size
        // ideally we'd use TBOs for bigger data but i didnt have time to impl
        uint32_t m_particleCount = 0;
    };
}