#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <iostream>

class EnabledSharedFromThisBase {};

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis : public EnabledSharedFromThisBase {
    template <class U>
    friend class SharedPtr;

public:
    SharedPtr<T> SharedFromThis() {
        return weak_this_.Lock();
    };

    SharedPtr<const T> SharedFromThis() const {
        return const_weak_this_.Lock();
    };

    WeakPtr<T> WeakFromThis() noexcept {
        return weak_this_;
    };

    WeakPtr<const T> WeakFromThis() const noexcept {
        return const_weak_this_;
    };

    // Пишу такую хуйню потому что friend не работает, надеюсь никто не будет это читать боже
    WeakPtr<T>& GetWeak() {
        return weak_this_;
    };

    WeakPtr<T>& GetConstWeak() {
        return const_weak_this_;
    }

    const WeakPtr<T>& GetWeak() const {
        return weak_this_;
    };

    const WeakPtr<T>& GetConstWeak() const {
        return const_weak_this_;
    }

    ~EnableSharedFromThis() {
    }

private:
    WeakPtr<T> weak_this_;
    WeakPtr<const T> const_weak_this_;
};

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    template <class U>
    friend class SharedPtr;

    template <class U>
    friend class EnableSharedFromThis;

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
        // std::cout << block_->GetCount() << " ! " << block_->WeakCount() << std::endl;
        AddStrongRef();
        // std::cout << block_->GetCount() << " ! " << block_->WeakCount() << std::endl;
        Assign(ptr);
        // std::cout << block_->GetCount() << " ! " << block_->WeakCount() << std::endl;
    };

    template <class U>
    explicit SharedPtr(U* ptr) : field_(ptr) {
        block_ = new ControlBlock<U>(ptr);
        AddStrongRef();
        Assign(ptr);
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

    BaseBlock*& GetBlock() {
        return block_;
    }
    // Promote WeakPtr
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.Expired()) {
            throw BadWeakPtr();
        }
        block_ = other.GetBlock();
        field_ = other.GetField();
        AddStrongRef();
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // operator=-s

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
        Reset();
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (block_) {
            // std::cout << block_->GetCount() << " " << block_->WeakCount() << std::endl;
            block_->AddWeakRef();
            DecStrongRef();
            // std::cout << block_->GetCount() << " " << block_->WeakCount() << std::endl;
            block_->DecWeakRef();
            CLear();
        }
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

    template <class U>
    void Helper(EnableSharedFromThis<U>* ptr) {
        WeakPtr<U> weak_this(*this);
        if constexpr (!std::is_const_v<U>) {
            ptr->GetWeak() = weak_this;
        } else {
            ptr->GetConstWeak() = weak_this;
        }
    }

    template <class U>
    void Assign(U* ptr) {
        if constexpr (std::is_convertible_v<U*, EnabledSharedFromThisBase*>) {
            Helper(ptr);
        }
    }

    T*& GetField() {
        return field_;
    }

    T* GetField() const {
        return field_;
    }

    BaseBlock* GetBlock() const {
        return block_;
    }

private:
    BaseBlock* block_ = nullptr;
    T* field_ = nullptr;

    template <typename U, typename... Args>
    friend SharedPtr<U> MakeShared(Args&&... args);
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.GetBlock() == right.GetBlock();
};

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    auto block = new Block<T>(std::forward<Args>(args)...);
    SharedPtr<T> obj;
    obj.block_ = block;
    obj.field_ = block->GetPtr();
    obj.AddStrongRef();
    obj.Assign(block->GetPtr());
    return std::move(obj);
}
