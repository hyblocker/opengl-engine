#include <stdio.h>

#include "engine/app.hpp"
#include "engine/renderer/scene_renderer.hpp"
#include "engine/renderer/scene_updater.hpp"
#include "engine/renderer/camera.hpp"
#include "engine/renderer/light.hpp"
#include "engine/renderer/mesh.hpp"
#include <hlsl++.h>

class ArkanoidLayer : public engine::ILayer {
public:
    ArkanoidLayer(gpu::DeviceManager* deviceManager, managers::AssetManager* assetManager);
    ~ArkanoidLayer() override;

    void update(double timeElapsed, double deltaTime) override;
    void render(double deltaTime) override;
    void event(engine::events::Event& event) override;
    void imguiDraw() override;

    // To handle app close properly
    void detach() override;
    
    inline void setActiveScene(render::Scene& newScene) {
        if (m_activeScene) {
            m_sceneUpdater.sleep(*m_activeScene);
        }
        m_activeScene = &newScene;
        m_sceneUpdater.start(*m_activeScene);
    }

private:
    bool windowResized(const engine::events::WindowResizeEvent& event);

    // Initialises the memory for the scenes
    // - setups scene hierarchy and attaches components
    void initScenes();
    void initMenuScene();
    void initGameScene(render::Scene& outScene);

    // Loads gpu resources into memory
    void loadGpuResources();

private:

    // Scene management
    // The currently active scene
    render::Scene* m_activeScene = nullptr;

    // Scenes loaded in memory
    // Ideally these would be serialised and we would just load from disk but for the purposes of the assignment this should suffice
    render::Scene m_menuScene;
    render::Scene m_gameSceneL1;
    render::Scene m_gameSceneL2;
    render::Scene m_gameSceneL3;
    render::Scene m_gameSceneL4;
    render::Scene m_gameSceneL5;

    // Gpu handles
    gpu::IShader* m_shaderModernOpaque = nullptr;
    gpu::IShader* m_shaderModernTransparent = nullptr;
    gpu::IShader* m_shaderClassic = nullptr;
    gpu::IShader* m_shaderParticle = nullptr;

    gpu::BlendStateHandle m_ballParticleBlendState;

    // Scene handlers
    render::SceneRenderer m_sceneRenderer;
    render::SceneUpdater m_sceneUpdater;

    std::vector<void*> m_sceneGarbage;

    // Debug temp vars
    bool m_doDrawDebugUi =
#if _DEBUG
        true;
#else
        false;
#endif
    bool m_doDrawDebugPhysics = false;
    void* m_selectedUiHierarchyElement = nullptr;
};