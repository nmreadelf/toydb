//
// Created by elf on 29/1/23.
//
#pragma once
#include <string>
#include <utility>

namespace toydb {
template <typename T> struct Status {
  bool ok_;
  union {
    std::string message_;
    T value_;
  };

  Status() : ok_(false), message_() {}

  ~Status() {
    if (ok_) {
      value_.~T();
    } else {
      message_.~basic_string();
    }
  }

  explicit Status(std::string msg) : ok_(false), message_(std::move(msg)) {}

  Status(Status const &rhs) : ok_(rhs.ok_) {
    if (ok_) {
      value_ = rhs.value_;
    } else {
      message_ = rhs.message_;
    }
  }

  Status &operator=(Status const &rhs) {
    if (ok_) {
      if (std::is_pointer_v<T>) {
        delete value_;
      } else {
        value_ = rhs.value_;
      }
      value_.~T();
    } else {
      message_.~basic_string();
    }
    ok_ = rhs.ok_;
    if (ok_) {
      value_ = rhs.value_;
    } else {
      message_ = rhs.message_;
    }

    return *this;
  }

  Status &operator=(Status &&rhs) noexcept {
    std::swap(value_, rhs.value_);
    std::swap(ok_, rhs.ok_);
    return *this;
  }

  bool ok() { return ok_; }
};

template <typename T> Status<T> OkWithValue(T v) {
  Status<T> s;
  s.ok_ = true;
  s.value_ = v;
  return s;
}
} // namespace toydb
