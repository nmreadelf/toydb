//
// Created by elf on 29/1/23.
//
#pragma once
#include <string>
#include <utility>

namespace toydb {

template <typename T> struct Status {
  enum Code { kOk = 0, kError = 1, kMoved = 2 };
  union {
    std::string message_;
    T value_;
  };

private:
  Code state_;

public:
  explicit Status(Code code = kError) : state_(code), message_() {}

  ~Status() {
    switch (state_) {
    case kOk:
      value_.~T();
      break;
    case kError:
      message_.~basic_string();
      break;
    }
  }

  explicit Status(std::string msg) : state_(kError), message_(std::move(msg)) {}

  Status(Status const &rhs) : state_(rhs.state_) {
    if (state_) {
      value_ = rhs.value_;
    } else {
      message_ = rhs.message_;
    }
  }

  Status &operator=(Status const &rhs) {
    switch (state_) {
    case kOk:
      if (std::is_pointer_v<T>) {
        delete value_;
      } else {
        value_.~T();
      }
      break;
    case kError:
      message_.~basic_string();
      break;
    }
    switch (rhs.state_) {
    case kOk:
      value_ = rhs.value_;
      break;
    case kError:
      message_ = rhs.message_;
      break;
    }
    state_ = rhs.state_;

    return *this;
  }

  Status &operator=(Status &&rhs) {
    switch (rhs.state_) {
    case kOk:
      value_.~T();
      value_ = std::move(rhs.value_);
      break;
    case kError:
      message_ = std::move(rhs.message_);
      break;
    default: {
      state_ = kError;
      message_ = "rhs is moved";
      return *this;
    }
    }
    state_ = std::move(rhs.state_);
    rhs.state_ = kMoved;
    return *this;
  }

  bool ok() { return state_ == kOk; }

  void SetMoved() { state_ == kMoved; }
};

template <typename T> Status<T> OkWithValue(T v) {
  Status<T> s(Status<T>::kOk);
  s.value_ = v;
  return s;
}
} // namespace toydb
