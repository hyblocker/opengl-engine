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
        GL_CHECK(glBindBuffer(getGlBufferType(handle->getDesc().type).glType, handle->getNativeObject()));
    }

    void GlDevice::setConstantBuffer(IBuffer* handle, uint32_t bindIndex) {
        ASSERT(handle != nullptr);
        ASSERT(handle->getDesc().type == gpu::BufferType::ConstantBuffer);
        GL_CHECK(glBindBufferBase(getGlBufferType(handle->getDesc().type).glType, bindIndex, handle->getNativeObject()));
    }

    void GlDevice::unbindConstantBuffer(IBuffer* handle, uint32_t bindIndex) {
        ASSERT(handle != nullptr);
        ASSERT(handle->getDesc().type == gpu::BufferType::ConstantBuffer);
        GL_CHECK(glBindBufferBase(getGlBufferType(handle->getDesc().type).glType, bindIndex, 0));
    }
    
    void GlDevice::unbindBuffer(IBuffer* handle) {
        ASSERT(handle != nullptr);
        GL_CHECK(glBindBuffer(getGlBufferType(handle->getDesc().type).glType, 0));
    }

    void GlDevice::writeBuffer(IBuffer* handle, size_t size, const void* data) {
        ASSERT(handle != nullptr);
        ASSERT(size > 0);
        if (handle->getDesc().type != BufferType::ConstantBuffer) {
            ASSERT(data != nullptr);
        }
        bindBuffer(handle);
        GL_CHECK(glBufferData(getGlBufferType(handle->getDesc().type).glType, size, data, getGlUsage(handle->getDesc().usage).glUsage));
    }

    void GlDevice::mapBuffer(IBuffer* buffer, uint32_t offset, size_t length, MapAccessFlags accessFlags, void** mappedDataPtr) {
        ASSERT(buffer != nullptr);
        ASSERT(mappedDataPtr != nullptr);

        void* bufferPtr = glMapBufferRange(getGlBufferType(buffer->getDesc().type).glType, offset, length, getGlAccessFlags(accessFlags).glFlag);
        GL_CHECK(;); // Error check because we have a return value above
        // @TODO: RE-ADD CHECK
        *mappedDataPtr = bufferPtr;
    }

    void GlDevice::unmapBuffer(IBuffer* buffer) {
        ASSERT(buffer != nullptr);

        GL_CHECK(glUnmapBuffer(getGlBufferType(buffer->getDesc().type).glType));
    }

    void GlDevice::setBufferBinding(IShader* shader, const std::string& name, uint32_t bindIndex) {
        ASSERT(shader != nullptr);
        ASSERT(!name.empty());
        ASSERT(bindIndex < m_maxUniformBufferBindings);

        uint32_t uniformBlockIndex = glGetUniformBlockIndex(shader->getNativeObject(), name.c_str());
        GL_CHECK(;);
        ASSERT(uniformBlockIndex != GL_INVALID_INDEX);
        ASSERT(uniformBlockIndex != GL_INVALID_OPERATION);
        GL_CHECK(glUniformBlockBinding(shader->getNativeObject(), uniformBlockIndex, bindIndex));
    }

    GlInputLayout::GlInputLayout() {}
    GlInputLayout::~GlInputLayout() {
        ASSERT(m_pointer != 0);
        glDeleteVertexArrays(1, &m_pointer);
        m_pointer = 0;
    }
}