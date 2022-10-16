#pragma once

#include <exception>

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

    bool WholeEmpty() const {
        return (!strong_ref_counter) && (!weak_reaf_counter);
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

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;
