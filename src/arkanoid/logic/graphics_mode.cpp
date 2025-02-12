#include "graphics_mode.hpp"

GraphicsModeTarget GraphicsMode::s_selectedGraphicsMode = GraphicsModeTarget::Modern;

constexpr float k_MODERN_TO_CLASSIC_MODE_SCALE = 0.1f;
constexpr float k_CLASSIC_TO_MODERN_MODE_SCALE = 10.0f;

void GraphicsMode::start() {
    m_pRenderer = (render::MeshRenderer*) getEntity()->findComponent(render::ComponentType::MeshRenderer);
    m_modernShader = m_pRenderer->material.shader;
}

void GraphicsMode::sleep() {

}

void GraphicsMode::update(float deltaTime) {
    if (m_lastGraphicsMode == s_selectedGraphicsMode) {
        return;
    }

    switch (s_selectedGraphicsMode) {
    case GraphicsModeTarget::Classic:
    {
        // now using classic mode
        hlslpp::float3 currentScale = getEntity()->transform.getScale();
        getEntity()->transform.setScale(hlslpp::float3(currentScale.xy, currentScale.z * k_MODERN_TO_CLASSIC_MODE_SCALE));
        m_pRenderer->material.shader = m_classicShader;
        break;
    }
    case GraphicsModeTarget::Modern:
    {
        // now using modern mode
        hlslpp::float3 currentScale = getEntity()->transform.getScale();
        getEntity()->transform.setScale(hlslpp::float3(currentScale.xy, currentScale.z * k_CLASSIC_TO_MODERN_MODE_SCALE));
        m_pRenderer->material.shader = m_modernShader;
        break;
    }
    default:
    {
        break;
    }
    }
    m_lastGraphicsMode = s_selectedGraphicsMode;
}