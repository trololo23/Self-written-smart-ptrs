#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t

template <class T>
struct Slug {
    Slug() = default;

    template <class V>
    Slug(Slug<V>&& other) {
    }

    template <class V>
    Slug& operator=(Slug<V>&& other) {
        return *this;
    }

    void operator()(T* ptr) {
        delete ptr;
    }
};

template <class T>
struct Slug<T[]> {
    Slug() = default;

    template <class V>
    Slug(Slug<V>&& other) {
    }

    template <class V>
    Slug& operator=(Slug<V>&& other) {
        return *this;
    }

    void operator()(T* ptr) {
        delete[] ptr;
    }
};

// Primary template
template <typename T, typename Deleter = Slug<T>>
class UniquePtr {
    template <class U, class BEB>
    friend class UniquePtr;
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : pair_(ptr, Deleter()) {
    }

    template <typename Del>
    UniquePtr(T* ptr, Del&& deleter) : pair_(ptr, std::forward<Del>(deleter)) {
    }

    template <class U, class Deleter2>
    UniquePtr(UniquePtr<U, Deleter2>&& other) noexcept {
        if (pair_.GetFirst() == other.pair_.GetFirst()) {
            return;
        }
        CorrectDelete();
        pair_.GetFirst() = other.pair_.GetFirst();
        pair_.GetSecond() = std::forward<Deleter2>(other.pair_.GetSecond());
        other.pair_.GetFirst() = nullptr;
        other.pair_.GetSecond() = Deleter2();
    }

    UniquePtr& operator=(const UniquePtr& other) = delete;

    UniquePtr(const UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    template <class U, class Deleter2>
    UniquePtr& operator=(UniquePtr<U, Deleter2>&& other) noexcept {
        if (pair_.GetFirst() == other.pair_.GetFirst()) {
            return *this;
        }
        CorrectDelete();
        pair_.GetFirst() = other.pair_.GetFirst();
        pair_.GetSecond() = std::forward<Deleter2>(other.pair_.GetSecond());
        other.pair_.GetSecond() = Deleter2();
        other.pair_.GetFirst() = nullptr;
        return *this;
    };

    void CorrectDelete() {
        auto temp = pair_.GetFirst();
        pair_.GetFirst() = nullptr;
        if (temp != nullptr) {
            pair_.GetSecond()(temp);
        }
    }

    UniquePtr& operator=(std::nullptr_t) {
        CorrectDelete();
        pair_.GetFirst() = nullptr;
        return *this;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        CorrectDelete();
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        auto tmp = pair_.GetFirst();
        pair_.GetFirst() = nullptr;
        return tmp;
    };

    void Reset(T* ptr = nullptr) {
        auto temp = pair_.GetFirst();
        pair_.GetFirst() = ptr;
        if (temp != nullptr) {
            pair_.GetSecond()(temp);
        }
    };

    void Swap(UniquePtr& other) {
        std::swap(pair_.GetFirst(), other.pair_.GetFirst());
        std::swap(pair_.GetSecond(), other.pair_.GetSecond());
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return pair_.GetFirst();
    };

    Deleter& GetDeleter() {
        return pair_.GetSecond();
    };

    const Deleter& GetDeleter() const {
        return pair_.GetSecond();
    };

    explicit operator bool() const {
        return pair_.GetFirst() != nullptr;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *pair_.GetFirst();
    };

    T* operator->() const {
        return pair_.GetFirst();
    };

private:
    CompressedPair<T*, Deleter> pair_;
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    explicit UniquePtr(T* ptr = nullptr) : pair_(ptr, Deleter()) {
    }

    template <typename Del>
    UniquePtr(T* ptr, Del&& deleter) : pair_(ptr, std::forward<Del>(deleter)) {
    }

    //    template <typename U>
    UniquePtr(UniquePtr&& other) noexcept {
        if (pair_.GetFirst() == other.pair_.GetFirst()) {
            return;
        }
        CorrectDelete();
        pair_.GetFirst() = other.pair_.GetFirst();
        other.pair_.GetFirst() = nullptr;
        pair_.GetSecond() = std::forward<Deleter>(other.pair_.GetSecond());
    }

    UniquePtr& operator=(const UniquePtr& other) = delete;

    UniquePtr(const UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    template <typename U>
    UniquePtr& operator=(UniquePtr<U>&& other) noexcept {
        if (pair_.GetFirst() == other.pair_.GetFirst()) {
            return *this;
        }
        CorrectDelete();
        pair_.GetFirst() = other.pair_.GetFirst();
        pair_.GetSecond() = std::forward<Deleter>(other.pair_.GetSecond());
        other.pair_.GetSecond() = Deleter();
        other.pair_.GetFirst() = nullptr;
        return *this;
    }

    void CorrectDelete() {
        auto temp = pair_.GetFirst();
        pair_.GetFirst() = nullptr;
        if (temp != nullptr) {
            pair_.GetSecond()(temp);
        }
    }

    UniquePtr& operator=(std::nullptr_t) {
        CorrectDelete();
        pair_.GetFirst() = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        CorrectDelete();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        auto tmp = pair_.GetFirst();
        pair_.GetFirst() = nullptr;
        return tmp;
    }

    void Reset(T* ptr = nullptr) {
        auto temp = pair_.GetFirst();
        pair_.GetFirst() = ptr;
        if (temp != nullptr) {
            pair_.GetSecond()(temp);
        }
    }

    void Swap(UniquePtr& other) {
        std::swap(pair_.GetFirst(), other.pair_.GetFirst());
        std::swap(pair_.GetSecond(), other.pair_.GetSecond());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return pair_.GetFirst();
    }

    Deleter& GetDeleter() {
        return pair_.GetSecond();
    }

    const Deleter& GetDeleter() const {
        return pair_.GetSecond();
    }

    explicit operator bool() const {
        return pair_.GetFirst() != nullptr;
    }

    T& operator[](size_t i) {
        return pair_.GetFirst()[i];
    }

    const T& operator[](size_t i) const {
        return pair_.GetFirst()[i];
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////
private:
    CompressedPair<T*, Deleter> pair_;
};
