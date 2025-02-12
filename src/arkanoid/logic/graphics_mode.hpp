#pragma once

#include "engine/renderer/scene_graph.hpp"
#include "engine/renderer/mesh.hpp"

enum class GraphicsModeTarget {
    Classic,
    Modern
};

class GraphicsMode : public render::IBehaviour {
public:
    GraphicsMode(render::Entity* parent, gpu::IShader* classicShader) : IBehaviour(parent), m_classicShader(classicShader) {}
    ~GraphicsMode() = default;

    void start() override;
    void sleep() override;
    void update(float deltaTime) override;

    static inline GraphicsModeTarget getGraphicsMode() { return s_selectedGraphicsMode; }
    static inline void setGraphicsMode(GraphicsModeTarget newMode) { s_selectedGraphicsMode = newMode; }

private:
    // static so that we only have a single one in the entire program, makes state management easier
    static GraphicsModeTarget s_selectedGraphicsMode;
    GraphicsModeTarget m_lastGraphicsMode = GraphicsModeTarget::Modern;

    gpu::IShader* m_classicShader = nullptr;
    gpu::IShader* m_modernShader = nullptr;

    render::MeshRenderer* m_pRenderer = nullptr;
};