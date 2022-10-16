#pragma once

#include <cstddef>  // for std::nullptr_t
#include <utility>  // for std::exchange / std::swap

class SimpleCounter {
public:
    size_t IncRef() {
        ++count_;
        return count_;
    };

    size_t DecRef() {
        --count_;
        return count_;
    };

    size_t RefCount() const {
        return count_;
    };

    SimpleCounter() = default;

    SimpleCounter(const SimpleCounter& other){};

    SimpleCounter& operator=(const SimpleCounter& other) {
        return *this;
    };

private:
    size_t count_ = 0;
};

struct DefaultDelete {
    template <typename T>
    static void Destroy(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
public:
    // Increase reference counter.
    void IncRef() {
        counter_.IncRef();
    };

    // Decrease reference counter.
    // Destroy object using Deleter when the last instance dies.
    void DecRef() {
        counter_.DecRef();
        if (!counter_.RefCount()) {
            Deleter::Destroy(static_cast<Derived*>(this));
        }
    };

    // Get current counter value (the number of strong references).
    size_t RefCount() const {
        return counter_.RefCount();
    };

private:
    Counter counter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
    template <typename Y>
    friend class IntrusivePtr;

public:
    // Constructors
    IntrusivePtr() = default;
    IntrusivePtr(std::nullptr_t) {
        ptr_ = nullptr;
    };

    IntrusivePtr(T* ptr) : ptr_(ptr) {
        Inc();
    };

    void Inc() {
        if (ptr_) {
            ptr_->IncRef();
        }
    }

    void Dec() {
        if (ptr_) {
            ptr_->DecRef();
            ptr_ = nullptr;
        }
    }

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other) {
        if (ptr_ != other.ptr_) {
            ptr_ = other.ptr_;
            Inc();
        }
    }

    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other) {
        if (ptr_ != other.ptr_) {
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
    }

    IntrusivePtr(const IntrusivePtr& other) {
        if (ptr_ != other.ptr_) {
            ptr_ = other.ptr_;
            Inc();
        }
    };

    IntrusivePtr(IntrusivePtr&& other) {
        if (ptr_ != other.ptr_) {
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
    };

    // `operator=`-s
    IntrusivePtr& operator=(const IntrusivePtr& other) {
        if (ptr_ == other.ptr_) {
            return *this;
        }
        Dec();
        ptr_ = other.ptr_;
        Inc();
        return *this;
    };

    IntrusivePtr& operator=(IntrusivePtr&& other) {
        if (ptr_ == other.ptr_) {
            return *this;
        }
        Dec();
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
        return *this;
    };

    // Destructor
    ~IntrusivePtr() {
        Reset();
    };

    // Modifiers
    void Reset() {
        Dec();
    };

    void Reset(T* ptr) {
        Dec();
        ptr_ = ptr;
        Inc();
    };

    void Swap(IntrusivePtr& other) {
        std::swap(ptr_, other.ptr_);
    };

    // Observers
    T* Get() const {
        return ptr_;
    };

    T& operator*() const {
        return *ptr_;
    };

    T* operator->() const {
        return ptr_;
    };

    size_t UseCount() const {
        if (!ptr_) {
            return 0;
        }
        return ptr_->RefCount();
    };

    explicit operator bool() const {
        return (ptr_ != nullptr);
    };

private:
    T* ptr_ = nullptr;
};

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    return std::move(IntrusivePtr<T>(new T(std::forward<Args>(args)...)));
}
