#pragma once

#include <inttypes.h>
#include <vector>
#include "ilayer.hpp"

namespace engine {

    class LayerStack {
    public:
        LayerStack() = default;
        ~LayerStack();

        void pushLayer(ILayer* layer);
        void pushOverlay(ILayer* overlay);
        // overlays are special layers which are drawn on top of normal layers. used for debugging layers such as imgui
        void popLayer(ILayer* layer);
        void popOverlay(ILayer* overlay);

        std::vector<ILayer*>::iterator begin() { return m_layers.begin(); }
        std::vector<ILayer*>::iterator end() { return m_layers.end(); }
        std::vector<ILayer*>::reverse_iterator rbegin() { return m_layers.rbegin(); }
        std::vector<ILayer*>::reverse_iterator rend() { return m_layers.rend(); }

        std::vector<ILayer*>::const_iterator begin() const { return m_layers.begin(); }
        std::vector<ILayer*>::const_iterator end() const { return m_layers.end(); }
        std::vector<ILayer*>::const_reverse_iterator rbegin() const { return m_layers.rbegin(); }
        std::vector<ILayer*>::const_reverse_iterator rend() const { return m_layers.rend(); }

    private:
        std::vector<ILayer*> m_layers;
        uint32_t m_layerInsertPointer = 0;
    };
}