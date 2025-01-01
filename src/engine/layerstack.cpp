#include "layerstack.hpp"
#include "core.hpp"

namespace engine {
    LayerStack::~LayerStack() {
        for (ILayer* layer : m_layers) {
            layer->detach();
            delete layer;
        }
    }

    void LayerStack::pushLayer(ILayer* layer) {
        ASSERT(layer != nullptr);
        m_layers.emplace(m_layers.begin() + m_layerInsertPointer, layer);
        m_layerInsertPointer++;
        layer->attach();
    }

    void LayerStack::pushOverlay(ILayer* overlay) {
        ASSERT(overlay != nullptr);
        m_layers.emplace_back(overlay);
        overlay->attach();
    }

    void LayerStack::popLayer(ILayer* layer) {
        ASSERT(layer != nullptr);
        
        auto iter = std::find(m_layers.begin(), m_layers.begin() + m_layerInsertPointer, layer);
        if (iter != m_layers.begin() + m_layerInsertPointer) {
            layer->detach();
            m_layers.erase(iter);
            m_layerInsertPointer--;
        }
    }

    void LayerStack::popOverlay(ILayer* overlay) {
        ASSERT(overlay != nullptr);

        auto iter = std::find(m_layers.begin(), m_layers.begin() + m_layerInsertPointer, overlay);
        if (iter != m_layers.begin() + m_layerInsertPointer) {
            overlay->detach();
            m_layers.erase(iter);
        }
    }
}