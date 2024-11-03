#include "device_manager.hpp"

#include "gl/gldevice.hpp"

namespace gpu {
    DeviceManager* DeviceManager::create() {
        DeviceManager* deviceManager = new DeviceManager();
        deviceManager->createDevice();
        return deviceManager;
    }

    bool DeviceManager::createDevice() {
        IDevice* device = new gl::GlDevice();
        m_device = DeviceHandle::Create(device);
        return true;
    }
}