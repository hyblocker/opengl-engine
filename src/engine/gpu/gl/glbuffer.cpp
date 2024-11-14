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
        glBindBuffer(getGlBufferType(handle->getDesc().type).glType, handle->getNativeObject());
    }

    void GlDevice::setConstantBuffer(IBuffer* handle, uint32_t bindIndex) {
        ASSERT(handle != nullptr);
        ASSERT(handle->getDesc().type == gpu::BufferType::ConstantBuffer);
        glBindBufferBase(getGlBufferType(handle->getDesc().type).glType, bindIndex, handle->getNativeObject());
    }
    
    void GlDevice::unbindBuffer(IBuffer* handle) {
        ASSERT(handle != nullptr);
        glBindBuffer(getGlBufferType(handle->getDesc().type).glType, 0);
    }

    void GlDevice::writeBuffer(IBuffer* handle, size_t size, const void* data) {
        ASSERT(handle != nullptr);
        ASSERT(size > 0);
        if (handle->getDesc().type != BufferType::ConstantBuffer) {
            ASSERT(data != nullptr);
        }
        bindBuffer(handle);
        glBufferData(getGlBufferType(handle->getDesc().type).glType, size, data, getGlUsage(handle->getDesc().usage).glUsage);
    }

    void GlDevice::mapBuffer(IBuffer* buffer, uint32_t offset, size_t length, MapAccessFlags accessFlags, void** mappedDataPtr) {
        ASSERT(buffer != nullptr);
        ASSERT(mappedDataPtr != nullptr);

        void* bufferPtr = glMapBufferRange(getGlBufferType(buffer->getDesc().type).glType, offset, length, getGlAccessFlags(accessFlags).glFlag);
        *mappedDataPtr = bufferPtr;
    }

    void GlDevice::unmapBuffer(IBuffer* buffer) {
        ASSERT(buffer != nullptr);

        glUnmapBuffer(getGlBufferType(buffer->getDesc().type).glType);
    }

    void GlDevice::setBufferBinding(IShader* shader, const std::string& name, uint32_t bindIndex) {
        ASSERT(shader != nullptr);
        ASSERT(!name.empty());
        ASSERT(bindIndex < GL_MAX_UNIFORM_BUFFER_BINDINGS);

        uint32_t uniformBlockIndex = glGetUniformBlockIndex(shader->getNativeObject(), name.c_str());
        ASSERT(uniformBlockIndex != GL_INVALID_INDEX);
        ASSERT(uniformBlockIndex != GL_INVALID_OPERATION);
        glUniformBlockBinding(shader->getNativeObject(), uniformBlockIndex, bindIndex);
    }

    GlInputLayout::GlInputLayout() {}
    GlInputLayout::~GlInputLayout() {
        ASSERT(m_pointer != 0);
        glDeleteVertexArrays(1, &m_pointer);
        m_pointer = 0;
    }
}