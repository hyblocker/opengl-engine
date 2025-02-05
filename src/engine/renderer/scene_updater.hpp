#pragma once

#include "scene_graph.hpp"

namespace render {
    // Handles updating all behaviours in a scene
    class SceneUpdater {
    public:
        void init();
        void start(const Scene& scene);
        void sleep(const Scene& scene);
        void render(const Scene& scene);
        void update(const Scene& scene, const float deltaTime);

        // for debugging, draws imgui tree
        void drawDebugSceneGraph(const Scene& scene, void** pSelectedEntity);
        void drawDebugInspector(const Scene& scene, void** pSelectedEntity);
    private:
        void start(const std::shared_ptr<Entity> entity);
        void sleep(const std::shared_ptr<Entity> entity);
        void render(const std::shared_ptr<Entity> entity);
        void update(const std::shared_ptr<Entity> entity, const float deltaTime);
        void physicsTick(const std::shared_ptr<Entity> entity, const float deltaTime, Scene::PhysicsParameters physicsParams);

        void drawDebugSceneGraphEntity(const std::string& sceneName, const std::shared_ptr<Entity> entity, void** pSelectedEntity);
    };
}