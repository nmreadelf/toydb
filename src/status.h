//
// Created by elf on 29/1/23.
//
#pragma once
#include <string>
#include <utility>

namespace toydb {
struct Error {
  std::string msg_;

  explicit Error(const std::string &m) : msg_(m) {}

  explicit Error(std::string &&m) : msg_(std::move(m)) {}

  Error &operator=(std::string &&m) {
    msg_ = m;

    return *this;
  }
};

template <typename T> struct Status {

  enum Code { kOk = 0, kError = 1, kMoved = 2 };

  union {
    Error error_;
    T value_;
  };

private:
  Code state_;

public:
  // explicit Status(Code code = kError) : state_(code), error_("") {}

  ~Status() {
    switch (state_) {
    case kOk:
      value_.~T();
      break;
    case kError:
      error_.~Error();
      break;
    }
  }

  // explicit Status(Error e) : state_(kError), error_(std::move(e)) {}

  // explicit Status(const Error &e) : state_(kError), error_(e) {}

  Status(Error &&e) : state_(kError), error_(std::move(e)) {}

  Status(const T &t) : state_(kOk), value_(t) {}

  Status(T &&t) : state_(kOk), value_(std::move(t)) {}

  Status &operator=(Error &&e) {
    switch (state_) {
    case kOk:
      if (std::is_pointer_v<T>) {
        delete value_;
      } else {
        value_.~T();
      }
      break;
    case kError:
      error_.~Error();
      break;
    }
    state_ = kError;
    error_ = std::move(e);

    return *this;
  }

  Status &operator=(T &&v) {
    switch (state_) {
    case kOk:
      if (std::is_pointer_v<T>) {
        delete value_;
      } else {
        value_.~T();
      }
      break;
    case kError:
      error_.~Error();
      break;
    }
    state_ = kError;
    value_ = std::move(v);

    return *this;
  }

  Status(Status const &rhs) : state_(rhs.state_) {
    if (state_) {
      value_ = rhs.value_;
    } else {
      error_ = rhs.error_;
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
      error_.~Error();
      break;
    }
    switch (rhs.state_) {
    case kOk:
      value_ = rhs.value_;
      break;
    case kError:
      error_ = rhs.error_;
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
      error_ = std::move(rhs.error_);
      break;
    default: {
      state_ = kError;
      error_ = "rhs is moved";
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

} // namespace toydb
