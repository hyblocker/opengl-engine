#include "arkanoid_layer.hpp"
#include "engine/input/input_manager.hpp"

#include <imgui.h>

void ArkanoidLayer::initScenes() {
    menuScene.layer = this;
    gameScene.layer = this;

    initMenuScene(menuScene);
    initGameScene(gameScene);
}

void ArkanoidLayer::loadGpuResources() {

    // Load shader for materials
    m_shaderModernOpaque = getAssetManager()->fetchShader({
        .graphicsState = {
            // opaque rendering
        },
        .vertShader = "vert.glsl",
        .fragShader = "frag.glsl",
        .debugName = "ModernOpaque"
        });
    getDevice()->setBufferBinding(m_shaderModernOpaque, "GeometryBuffer", 0);
    getDevice()->setBufferBinding(m_shaderModernOpaque, "MaterialBuffer", 1);
    getDevice()->setBufferBinding(m_shaderModernOpaque, "LightsBuffer", 2);

    m_shaderModernTransparent = getAssetManager()->fetchShader({
        .graphicsState = {
            // transparent rendering
            // @TODO:
        },
        .vertShader = "vert.glsl",
        .fragShader = "frag.glsl",
        .debugName = "ModernAlphaBlend"
        });
    getDevice()->setBufferBinding(m_shaderModernTransparent, "GeometryBuffer", 0);
    getDevice()->setBufferBinding(m_shaderModernTransparent, "MaterialBuffer", 1);
    getDevice()->setBufferBinding(m_shaderModernTransparent, "LightsBuffer", 2);

    m_shaderClassic = getAssetManager()->fetchShader({
        .graphicsState = {
            // opaque rendering
        },
        .vertShader = "classic_vert.glsl",
        .fragShader = "classic_frag.glsl",
        .debugName = "Classic"
        });
    getDevice()->setBufferBinding(m_shaderClassic, "GeometryBuffer", 0);
    getDevice()->setBufferBinding(m_shaderClassic, "MaterialBuffer", 1);
    getDevice()->setBufferBinding(m_shaderClassic, "LightsBuffer", 2);

    m_shaderParticle = getAssetManager()->fetchShader({
        .graphicsState = {
            // opaque rendering
            .depthWrite = false,
            .faceCullingMode = gpu::FaceCullMode::Never,
        },
        .vertShader = "particle_vert.glsl",
        .fragShader = "particle_frag.glsl",
        .debugName = "Particles"
        });
    getDevice()->setBufferBinding(m_shaderParticle, "GeometryBuffer", 0);
    getDevice()->setBufferBinding(m_shaderParticle, "MaterialBuffer", 1);
    getDevice()->setBufferBinding(m_shaderParticle, "LightsBuffer", 2);
    getDevice()->setBufferBinding(m_shaderParticle, "ParticleBuffer", 3);
}