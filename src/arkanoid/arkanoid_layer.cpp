#include "arkanoid_layer.hpp"
#include "engine/input/input_manager.hpp"

#include <imgui.h>

void ArkanoidLayer::event(engine::events::Event& event) {
    engine::events::EventDispatcher dispatcher(event);
    dispatcher.dispatch<engine::events::WindowResizeEvent>(EVENT_BIND_FUNC(ArkanoidLayer::windowResized));
}

ArkanoidLayer::ArkanoidLayer(gpu::DeviceManager* deviceManager, managers::AssetManager* assetManager)
    : ILayer(deviceManager, assetManager, "ArkanoidLayer") {

    // Initial frame for renderer
    getDevice()->clearColor({ 0, 0, 0, 1 });
    getDevice()->setViewport({
        .left = 0,
        .right = engine::App::getInstance()->getWindow()->getWidth(),
        .top = 0,
        .bottom = engine::App::getInstance()->getWindow()->getHeight(),
    });

    // Backend warmup
    m_sceneUpdater.init();
    m_sceneRenderer.init(getDevice(), getAssetManager());

    loadGpuResources();
    initScenes();

    // Start on the menu scene
    // setActiveScene(m_menuScene);
    setActiveScene(m_gameScene);

    // @TODO: init post processing stack

    auto fbo = getDevice()->makeFramebuffer({
        .colorDesc = {
            .width = engine::App::getInstance()->getWindow()->getWidth(),
            .height = engine::App::getInstance()->getWindow()->getHeight(),
            .samples = 4,
            .format = gpu::TextureFormat::RGB10_A2,
        },
        .depthStencilDesc = {
            .width = engine::App::getInstance()->getWindow()->getWidth(),
            .height = engine::App::getInstance()->getWindow()->getHeight(),
            .samples = 1,
            .format = gpu::TextureFormat::Depth24_Stencil8,
        },
        .hasDepth = true,
        .debugName = "FBO"
    });
}

ArkanoidLayer::~ArkanoidLayer() {
    for (size_t i = 0; i < m_sceneGarbage.size(); i++) {
        delete m_sceneGarbage[i];
        m_sceneGarbage[i] = nullptr;
    }
}

void ArkanoidLayer::update(double timeElapsed, double deltaTime) {
    if (m_activeScene == nullptr) {
        LOG_ERROR("Scene is null! Skipping tick...");
        return;
    }

    if (engine::input::InputManager::getInstance()->keyReleased(engine::input::Keycode::Tilde)) {
        m_doDrawDebugUi = !m_doDrawDebugUi;
    }

    m_sceneUpdater.update(*m_activeScene, deltaTime);

    // @TODO: Box2D state update

}

void ArkanoidLayer::render(double deltaTime) {

    if (m_activeScene == nullptr) {
        LOG_ERROR("Scene is null! Skipping frame...");
        return;
    }

    getDevice()->clearColor({ 0, 0, 0, 1 });

    float backbufferAspect = engine::App::getInstance()->getWindow()->getWidth() / (float)engine::App::getInstance()->getWindow()->getHeight();

    m_sceneUpdater.render(*m_activeScene);
    m_sceneRenderer.draw(*m_activeScene, backbufferAspect);
}

bool ArkanoidLayer::windowResized(const engine::events::WindowResizeEvent& event) {

    getDevice()->setViewport({
        .left = 0,
        .right = event.width,
        .top = 0,
        .bottom = event.height,
    });

    // @TODO: Signal post-processing stack resize fbos

    return false;
}

void ArkanoidLayer::detach() {
    if (m_activeScene) {
        m_sceneUpdater.sleep(*m_activeScene);
    }
}

void ArkanoidLayer::imguiDraw() {

    if (m_doDrawDebugUi) {

        ImGui::ShowDemoWindow();

        ImGui::Begin("Scene Hierarchy");
        m_sceneUpdater.drawDebugSceneGraph(*m_activeScene, &m_selectedUiHierarchyElement);
        ImGui::End();

        ImGui::Begin("Inspector");
        m_sceneUpdater.drawDebugInspector(*m_activeScene, &m_selectedUiHierarchyElement);
        ImGui::End();
    }
}