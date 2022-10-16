#pragma once

#pragma once
#include <type_traits>
#include <memory>

template <class F, class S, bool FirstCompressed = (std::is_empty_v<F> && !std::is_final_v<F>),
          bool SecondCompressed = (std::is_empty_v<S> && !std::is_final_v<S>),
          bool IsNotBase = (!std::is_base_of_v<F, S> && !std::is_base_of_v<F, S>)>
class CompressedPair {};

template <class F, class S>
class CompressedPair<F, S, true, true, true> : F, S {
public:
    CompressedPair(){};

    CompressedPair(const F& first, const S& second) : F(first), S(second){};

    CompressedPair(const F& first, S&& second) : F(first), S(std::move(second)){};

    CompressedPair(F&& first, const S& second) : F(std::move(first)), S(second){};

    CompressedPair(F&& first, S&& second) : F(std::move(first)), S(std::move(second)){};

    F& GetFirst() {
        return static_cast<F&>(*this);
    }

    S& GetSecond() {
        return static_cast<S&>(*this);
    }

    const F& GetFirst() const {
        return static_cast<const F&>(*this);
    };

    const S& GetSecond() const {
        return static_cast<const S&>(*this);
    }
};

template <class F, class S>
class CompressedPair<F, S, true, true, false> : F {
public:
    CompressedPair() : second_(){};

    CompressedPair(const F& first, const S& second) : F(first), second_(second){};

    CompressedPair(const F& first, S&& second) : F(first), second_(std::move(second)){};

    CompressedPair(F&& first, const S& second) : F(std::move(first)), second_(second){};

    CompressedPair(F&& first, S&& second) : F(std::move(first)), second_(std::move(second)){};

    F& GetFirst() {
        return static_cast<F&>(*this);
    }

    S& GetSecond() {
        return second_;
    }

    const F& GetFirst() const {
        return static_cast<const F&>(*this);
    };

    const S& GetSecond() const {
        return second_;
    }

private:
    S second_;
};

template <class F, class S>
class CompressedPair<F, S, true, false, true> : F {
public:
    CompressedPair() : second_(){};

    CompressedPair(const F& first, const S& second) : F(first), second_(second){};

    CompressedPair(const F& first, S&& second) : F(first), second_(std::move(second)){};

    CompressedPair(F&& first, const S& second) : F(std::move(first)), second_(second){};

    CompressedPair(F&& first, S&& second) : F(std::move(first)), second_(std::move(second)){};

    F& GetFirst() {
        return static_cast<F&>(*this);
    }

    S& GetSecond() {
        return second_;
    }

    const F& GetFirst() const {
        return static_cast<const F&>(*this);
    };

    const S& GetSecond() const {
        return second_;
    }

private:
    S second_;
};

template <class F, class S>
class CompressedPair<F, S, true, false, false> : F {
public:
    CompressedPair() : second_(){};

    CompressedPair(const F& first, const S& second) : F(first), second_(second){};

    CompressedPair(const F& first, S&& second) : F(first), second_(std::move(second)){};

    CompressedPair(F&& first, const S& second) : F(std::move(first)), second_(second){};

    CompressedPair(F&& first, S&& second) : F(std::move(first)), second_(std::move(second)){};

    F& GetFirst() {
        return static_cast<F&>(*this);
    }

    S& GetSecond() {
        return second_;
    }

    const F& GetFirst() const {
        return static_cast<const F&>(*this);
    };

    const S& GetSecond() const {
        return second_;
    }

private:
    S second_;
};

template <class F, class S>
class CompressedPair<F, S, false, true, true> : S {
public:
    CompressedPair() : first_(){};

    CompressedPair(const F& first, const S& second) : first_(first), S(second){};

    CompressedPair(const F& first, S&& second) : first_(first), S(std::move(second)){};

    CompressedPair(F&& first, const S& second) : first_(std::move(first)), S(second){};

    CompressedPair(F&& first, S&& second) : first_(std::move(first)), S(std::move(second)){};

    F& GetFirst() {
        return first_;
    }

    S& GetSecond() {
        return static_cast<S&>(*this);
    }

    const F& GetFirst() const {
        return first_;
    };

    const S& GetSecond() const {
        return static_cast<const S&>(*this);
    }

private:
    F first_;
};

template <class F, class S>
class CompressedPair<F, S, false, true, false> : S {
public:
    CompressedPair() : first_(){};

    CompressedPair(const F& first, const S& second) : first_(first), S(second){};

    CompressedPair(const F& first, S&& second) : first_(first), S(std::move(second)){};

    CompressedPair(F&& first, const S& second) : first_(std::move(first)), S(second){};

    CompressedPair(F&& first, S&& second) : first_(std::move(first)), S(std::move(second)){};

    F& GetFirst() {
        return first_;
    }

    S& GetSecond() {
        return static_cast<S&>(*this);
    }

    const F& GetFirst() const {
        return first_;
    };

    const S& GetSecond() const {
        return static_cast<const S&>(*this);
    }

private:
    F first_;
};

template <class F, class S>
class CompressedPair<F, S, false, false, true> {
public:
    CompressedPair() : first_(), second_(){};

    CompressedPair(const F& first, const S& second) : first_(first), second_(second){};

    CompressedPair(const F& first, S&& second) : first_(first), second_(std::move(second)){};

    CompressedPair(F&& first, const S& second) : first_(std::move(first)), second_(second){};

    CompressedPair(F&& first, S&& second) : first_(std::move(first)), second_(std::move(second)){};

    F& GetFirst() {
        return first_;
    }

    S& GetSecond() {
        return second_;
    }

    const F& GetFirst() const {
        return first_;
    };

    const S& GetSecond() const {
        return second_;
    }

private:
    F first_;
    S second_;
};

template <class F, class S>
class CompressedPair<F, S, false, false, false> {
public:
    CompressedPair() : first_(), second_(){};

    CompressedPair(const F& first, const S& second) : first_(first), second_(second){};

    CompressedPair(const F& first, S&& second) : first_(first), second_(std::move(second)){};

    CompressedPair(F&& first, const S& second) : first_(std::move(first)), second_(second){};

    CompressedPair(F&& first, S&& second) : first_(std::move(first)), second_(std::move(second)){};

    F& GetFirst() {
        return first_;
    }

    S& GetSecond() {
        return second_;
    }

    const F& GetFirst() const {
        return first_;
    };

    const S& GetSecond() const {
        return second_;
    }

private:
    F first_;
    S second_;
};
