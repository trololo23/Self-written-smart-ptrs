#pragma once

#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
    template <class U>
    friend class SharedPtr;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() = default;

    void AddWeakRef() {
        if (!block_) {
            return;
        }
        block_->AddWeakRef();
    }

    void DecWeakRef() {
        if (!block_) {
            return;
        }
        block_->DecWeakRef();
        if (block_->WholeEmpty()) {
            delete block_;
            block_ = nullptr;
            field_ = nullptr;
        }
    }

    WeakPtr(const WeakPtr& other) {
        block_ = other.block_;
        field_ = other.field_;
        AddWeakRef();
    }

    WeakPtr(WeakPtr&& other) noexcept {
        block_ = other.block_;
        field_ = other.field_;
        other.block_ = nullptr;
        other.field_ = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) {
        block_ = other.GetBlock();
        field_ = other.GetField();
        AddWeakRef();
    }

    template <class U>
    WeakPtr(const SharedPtr<U>& other) {
        block_ = other.GetBlock();
        field_ = other.GetField();
        AddWeakRef();
    }

    WeakPtr& operator=(const SharedPtr<T>& other) {
        DecWeakRef();
        block_ = other.GetBlock();
        field_ = other.GetField();
        AddWeakRef();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        DecWeakRef();
        block_ = other.block_;
        field_ = other.field_;
        AddWeakRef();
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) noexcept {
        DecWeakRef();
        block_ = other.block_;
        field_ = other.field_;
        other.block_ = nullptr;
        other.field_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        DecWeakRef();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        DecWeakRef();
        block_ = nullptr;
        field_ = nullptr;
    }

    void Swap(WeakPtr& other) {
        std::swap(block_, other.block_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (block_) {
            return block_->GetCount();
        }
        return 0;
    }

    bool Expired() const {
        if (!block_) {
            return true;
        }
        return block_->IsEmpty();
    }

    SharedPtr<T> Lock() const {
        if (!block_ || block_->IsEmpty()) {
            return std::move(SharedPtr<T>());
        }
        SharedPtr<T> obj;
        obj.GetField() = field_;
        obj.GetBlock() = block_;
        obj.AddStrongRef();
        return std::move(obj);
    }

    T* GetField() const {
        return field_;
    }

    BaseBlock* GetBlock() const {
        return block_;
    }

private:
    T* field_ = nullptr;
    BaseBlock* block_ = nullptr;
};
