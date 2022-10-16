#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t

class BaseBlock {
public:
    void AddStrongRef() {
        ++strong_ref_counter;
    }

    virtual void DecStrongRef() {
        --strong_ref_counter;
    }

    void AddWeakRef() {
        ++weak_reaf_counter;
    }

    virtual void DecWeakRef() {
        --weak_reaf_counter;
    }

    int GetCount() const {
        return strong_ref_counter;
    }

    bool IsEmpty() const {
        return (strong_ref_counter == 0);
    }

    int WeakCount() const {
        return weak_reaf_counter;
    }

    virtual ~BaseBlock(){};

protected:
    int strong_ref_counter = 0;
    int weak_reaf_counter = 0;
};

template <class T>
class ControlBlock : public BaseBlock {
public:
    ControlBlock(T* ptr) : ptr_(ptr) {
    }

    void DecStrongRef() override {
        --strong_ref_counter;
        if (!strong_ref_counter) {
            delete ptr_;
            ptr_ = nullptr;
        }
    }

    void DecWeakRef() override {
        --weak_reaf_counter;
        if (!strong_ref_counter && !weak_reaf_counter) {
            delete ptr_;
        }
    }

    ~ControlBlock() override {
        if (ptr_) {
            delete ptr_;
        }
    }

private:
    T* ptr_;
};

template <class T>
class Block : public BaseBlock {
public:
    template <typename... Args>
    Block(Args&&... args) {
        new (&storage_) T(std::forward<Args>(args)...);
    }

    T* GetPtr() {
        return reinterpret_cast<T*>(&storage_);
    }

    ~Block() = default;

    void DecStrongRef() override {
        --strong_ref_counter;
        if (!strong_ref_counter) {
            reinterpret_cast<T*>(&storage_)->~T();
        }
    }

    void DecWeakRef() override {
        --weak_reaf_counter;
    }

private:
    std::aligned_storage_t<sizeof(T), alignof(T)> storage_;
};

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    template <class U>
    friend class SharedPtr;
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() {
    }

    SharedPtr(std::nullptr_t) {
        field_ = nullptr;
        block_ = nullptr;
    };

    explicit SharedPtr(T* ptr) : field_(ptr) {
        block_ = new ControlBlock<T>(ptr);
        AddStrongRef();
    };

    template <class U>
    explicit SharedPtr(U* ptr) : field_(ptr) {
        block_ = new ControlBlock<U>(ptr);
        AddStrongRef();
    }

    SharedPtr(const SharedPtr& other) {
        if (field_ != other.field_) {
            block_ = other.block_;
            field_ = other.field_;
            AddStrongRef();
        }
    };

    template <class U>
    SharedPtr(const SharedPtr<U>& other) {
        if (field_ != other.field_) {
            block_ = other.block_;
            field_ = other.field_;
            AddStrongRef();
        }
    }

    SharedPtr(SharedPtr&& other) {
        DecStrongRef();
        block_ = other.block_;
        field_ = other.field_;
        other.block_ = nullptr;
        other.field_ = nullptr;
    };

    template <class U>
    SharedPtr(SharedPtr<U>&& other) {
        DecStrongRef();
        block_ = other.block_;
        field_ = other.field_;
        other.block_ = nullptr;
        other.field_ = nullptr;
    }

    void AddStrongRef() {
        if (block_) {
            block_->AddStrongRef();
        }
    }

    void DecStrongRef() {
        if (block_) {
            block_->DecStrongRef();
        }
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) : field_(ptr) {
        block_ = other.GetBlock();
        AddStrongRef();
    }

    BaseBlock* GetBlock() const {
        return block_;
    }
    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other){

    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        if (field_ != other.field_) {
            DecStrongRef();
            CLear();
            block_ = other.block_;
            field_ = other.field_;
            AddStrongRef();
        }
        return *this;
    };

    template <class U>
    SharedPtr& operator=(const SharedPtr<U>& other) {
        if (field_ != other.field_) {
            DecStrongRef();
            CLear();
            block_ = other.block_;
            field_ = other.field_;
            AddStrongRef();
        }
        return *this;
    }

    void CLear() {
        if (block_ && block_->IsEmpty() && !block_->WeakCount()) {
            delete block_;
        }
        block_ = nullptr;
        field_ = nullptr;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        DecStrongRef();
        CLear();
        block_ = other.block_;
        field_ = other.field_;
        other.block_ = nullptr;
        other.field_ = nullptr;
        return *this;
    };

    template <class U>
    SharedPtr& operator=(SharedPtr<U>&& other) {
        DecStrongRef();
        CLear();
        block_ = other.block_;
        field_ = other.field_;
        other.block_ = nullptr;
        other.field_ = nullptr;
        return *this;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        DecStrongRef();
        CLear();
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        DecStrongRef();
        CLear();
        field_ = nullptr;
        block_ = nullptr;
    };

    void Reset(T* ptr) {
        DecStrongRef();
        CLear();
        field_ = ptr;
        block_ = new ControlBlock<T>(ptr);
        AddStrongRef();
    };

    template <class U>
    void Reset(U* ptr) {
        DecStrongRef();
        CLear();
        field_ = ptr;
        block_ = new ControlBlock<U>(ptr);
        AddStrongRef();
    }

    void Swap(SharedPtr& other) {
        std::swap(block_, other.block_);
        std::swap(field_, other.field_);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return field_;
    };

    T& operator*() const {
        return *field_;
    };

    T* operator->() const {
        return field_;
    };

    size_t UseCount() const {
        if (!block_) {
            return 0;
        }
        return block_->GetCount();
    };

    explicit operator bool() const {
        return (field_ != nullptr);
    };

private:
    BaseBlock* block_ = nullptr;
    T* field_ = nullptr;

    template <typename U, typename... Args>
    friend SharedPtr<U> MakeShared(Args&&... args);
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right);

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    auto block = new Block<T>(std::forward<Args>(args)...);
    SharedPtr<T> obj;
    obj.block_ = block;
    obj.field_ = block->GetPtr();
    obj.AddStrongRef();
    return std::move(obj);
}

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept;
    WeakPtr<const T> WeakFromThis() const noexcept;
};
