#pragma once

#include "scene_graph.hpp"

namespace render {
    class Renderer {
    public:
        void init();
        void draw(const Scene& scene);
    private:
        void draw(const Entity& entity);
        void drawSkybox(const Scene& scene, Camera* camera);


    };
}