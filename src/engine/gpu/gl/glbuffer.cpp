#include "gldevice.hpp"
#include "engine/core.hpp"
#include "engine/gpu/gl/glmappings.hpp"

#include <glad/glad.h>

namespace gpu::gl {
    GlBuffer::GlBuffer(gpu::BufferDesc bufferDesc)
        : m_bufferDesc(bufferDesc) {}

    GlBuffer::~GlBuffer() {
        ASSERT(m_pointer != 0);
        glDeleteBuffers(1, &m_pointer);
        m_pointer = 0;
    }

    void GlDevice::bindBuffer(IBuffer* handle) {
        ASSERT(handle != nullptr);
        if (m_currentBuffers[(uint32_t)handle->getDesc().type] != handle->getNativeObject()) {
            glBindBuffer(getGlBufferType(handle->getDesc().type).glType, handle->getNativeObject());
            m_currentBuffers[(uint32_t)handle->getDesc().type] = handle->getNativeObject();
        }
    }

    void GlDevice::writeBuffer(IBuffer* handle, size_t size, const void* data) {
        ASSERT(handle != nullptr);
        ASSERT(size > 0);
        ASSERT(data != nullptr);
        bindBuffer(handle);
        glBufferData(getGlBufferType(handle->getDesc().type).glType, size, data, getGlUsage(handle->getDesc().usage).glUsage);
    }

    GlInputLayout::GlInputLayout() {}
    GlInputLayout::~GlInputLayout() {
        ASSERT(m_pointer != 0);
        glDeleteVertexArrays(1, &m_pointer);
        m_pointer = 0;
    }
}