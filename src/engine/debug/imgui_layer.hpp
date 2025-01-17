#pragma once

#include "engine/ilayer.hpp"
#include "engine/events/event.hpp"

namespace engine {
    class ImguiLayer : public ILayer {
    public:

        ImguiLayer(gpu::DeviceManager* deviceManager, managers::AssetManager* assetManager);
        ~ImguiLayer() = default;

        virtual void attach() override;
        virtual void detach() override;
        virtual void event(events::Event& event) override;

        void begin();
        void end();

        void blockEvents(bool block) { m_eventsBlocked = block; }

        uint32_t getActiveWidgetId() const;

    private:
        bool m_eventsBlocked = true;
    };
}