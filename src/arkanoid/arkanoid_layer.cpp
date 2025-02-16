#include "arkanoid_layer.hpp"
#include "engine/input/input_manager.hpp"
#include "arkanoid/logic/level_handler.hpp"

#include <imgui.h>

#include "b2debug/debug_draw.hpp"

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
    // setActiveScene(m_gameScene);
    setActiveScene(m_menuScene);

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
    m_sceneUpdater.shutdown();
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

    // ` toggles ImGUI
    // ` + P toggles physics debugging
    if (engine::input::InputManager::getInstance()->keyReleased(engine::input::Keycode::Tilde)) {
        m_doDrawDebugUi = !m_doDrawDebugUi;
    }
    if (engine::input::InputManager::getInstance()->keyDown(engine::input::Keycode::Tilde) && engine::input::InputManager::getInstance()->keyReleased(engine::input::Keycode::P)) {
        m_doDrawDebugPhysics = !m_doDrawDebugPhysics;
    }

    m_sceneUpdater.update(*m_activeScene, deltaTime);

}

void ArkanoidLayer::render(double deltaTime) {

    if (m_activeScene == nullptr) {
        LOG_ERROR("Scene is null! Skipping frame...");
        return;
    }

    getDevice()->clearColor({ 0, 0, 0, 1 });

    float backbufferAspect = engine::App::getInstance()->getWindow()->getWidth() / (float)engine::App::getInstance()->getWindow()->getHeight();

    m_sceneUpdater.render(*m_activeScene);
    m_sceneRenderer.draw(*m_activeScene, backbufferAspect, deltaTime);
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

#if _DEBUG
    if (m_doDrawDebugUi) {

        m_sceneUpdater.imgui(*m_activeScene);

        // ImGui::ShowDemoWindow();

        ImGui::Begin("Scene Hierarchy");
        m_sceneUpdater.drawDebugSceneGraph(*m_activeScene, &m_selectedUiHierarchyElement);
        ImGui::End();

        ImGui::Begin("Inspector");
        m_sceneUpdater.drawDebugInspector(*m_activeScene, &m_selectedUiHierarchyElement);
        ImGui::End();

        if (m_doDrawDebugPhysics) {
            
            ImGui::Begin("Physics");
            ImGui::Text("Physics view enabled");
            static float zoom = 2000.0f;
            ImGui::DragFloat("Zoom", &zoom);

            ImGui::Checkbox("Use bounds", &g_draw.m_debugDraw.useDrawingBounds);
            ImGui::Checkbox("Shapes", &g_draw.m_debugDraw.drawShapes);
            ImGui::Checkbox("Joints", &g_draw.m_debugDraw.drawJoints);
            ImGui::Checkbox("Joints Extras", &g_draw.m_debugDraw.drawJointExtras);
            ImGui::Checkbox("AABBs", &g_draw.m_debugDraw.drawAABBs);
            ImGui::Checkbox("Mass", &g_draw.m_debugDraw.drawMass);
            ImGui::Checkbox("Contacts", &g_draw.m_debugDraw.drawContacts);
            ImGui::Checkbox("Colours", &g_draw.m_debugDraw.drawGraphColors);
            ImGui::Checkbox("Normals", &g_draw.m_debugDraw.drawContactNormals);
            ImGui::Checkbox("Impulses", &g_draw.m_debugDraw.drawContactImpulses);
            ImGui::Checkbox("Friction", &g_draw.m_debugDraw.drawFrictionImpulses);
            ImGui::End();

            getDevice()->debugMarkerPush("b2debug");
            // HACK
            LevelHandler* comp = (LevelHandler*)((m_activeScene->findNamedEntity("GameManager"))->findComponent(render::ComponentType::UserBehaviour));
            // m_sceneUpdater.drawPhysicsDebug(*m_activeScene);

            render::IComponent* camera = m_activeScene->findComponent(render::ComponentType::Camera);
            g_camera.m_center.x = camera->getEntity()->transform.getPosition().x;
            g_camera.m_center.y = camera->getEntity()->transform.getPosition().y;
            g_camera.m_zoom = zoom;
            b2World_Draw(comp->getWorldId(), &g_draw.m_debugDraw);
            g_draw.Flush();

            getDevice()->debugMarkerPop();
        }
    }
#endif
}