#include "arkanoid_layer.hpp"
#include "engine/input/input_manager.hpp"

#include <imgui.h>

void ArkanoidLayer::initScenes() {
    initMenuScene();
    initGameScene(m_gameSceneL1);
    // initGameScene(m_gameSceneL2);
    // initGameScene(m_gameSceneL3);
    // initGameScene(m_gameSceneL4);
    // initGameScene(m_gameSceneL5);
}

void ArkanoidLayer::loadGpuResources() {

    // Load shader for materials
    // @TODO: Replace with proper generic shader
    m_shader = getAssetManager()->fetchShader({
        .graphicsState = {
            // opaque rendering
        },
        .vertShader = "vert.glsl",
        .fragShader = "frag.glsl",
        .debugName = "Simple"
        });
    getDevice()->setBufferBinding(m_shader, "GeometryBuffer", 0);
    getDevice()->setBufferBinding(m_shader, "MaterialBuffer", 1);
    getDevice()->setBufferBinding(m_shader, "LightsBuffer", 2);
}