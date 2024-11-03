#pragma once

// Mostly a copy of std::shared_ptr
namespace engine {
    template<class T>
    class RefCounter {
    private:
        T* m_ptr = nullptr;
        unsigned long* m_refCount = nullptr;
    public:
        // Default ctor
        RefCounter() : m_ptr(nullptr), m_refCount(new unsigned long(1)) {}

        // Release on free
        ~RefCounter() {
            if (m_refCount && --(*m_refCount) == 0) {
                delete m_ptr;
                delete m_refCount;
            }
        }

        // Copy semantics
        RefCounter(const RefCounter& obj) : m_ptr(obj.m_ptr), m_refCount(obj.m_refCount) {
            if (m_refCount) {
                ++(*m_refCount);
            }
        }

        RefCounter& operator=(const RefCounter& obj) {
            if (this != &obj) {
                if (m_refCount && --(*m_refCount) == 0) {
                    delete m_ptr;
                    delete m_refCount;
                }
                m_ptr = obj.m_ptr;
                m_refCount = obj.m_refCount;
                if (m_refCount) {
                    ++(*m_refCount);
                }
            }
            return *this;
        }

        // Move semantics
        RefCounter(RefCounter&& other) noexcept : m_ptr(other.m_ptr), m_refCount(other.m_refCount) {
            other.m_ptr = nullptr;
            other.m_refCount = nullptr;
        }

        RefCounter& operator=(RefCounter&& other) noexcept {
            if (this != &other) {
                if (m_refCount && --(*m_refCount) == 0) {
                    delete m_ptr;
                    delete m_refCount;
                }
                m_ptr = other.m_ptr;
                m_refCount = other.m_refCount;
                other.m_ptr = nullptr;
                other.m_refCount = nullptr;
            }
            return *this;
        }

        // Smart pointer
        operator T* () const { return m_ptr; }
        T* operator->() const noexcept { return m_ptr; }

        // Quality of life functions
        unsigned long GetCount() const { return *m_refCount; }
        [[nodiscard]] T* Get() const noexcept { return m_ptr; }
        [[nodiscard]] T* const* GetAddressOf() const noexcept { return &m_ptr; }
        T* Detach() noexcept { T* ptr = m_ptr; m_ptr = nullptr; return ptr; }
        void Attach(T* other) { m_ptr = other; }
        static RefCounter<T> Create(T* other) { RefCounter<T> ptr; ptr.Attach(other); return ptr; }
    };
}