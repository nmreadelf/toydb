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

  explicit Error(const Error &e) = default;
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

  // Status(Error e) : state_(kError), error_(std::move(e)) {}

  Status(Error &e) : state_(kError), error_(e) {}

  Status(Error &&e) : state_(kError), error_(std::move(e)) {}

  Status(const T &t) : state_(kOk), value_(t) {}

  Status(T &&t) : state_(kOk), value_(std::move(t)) {}

  Status &operator=(Error &&e) {
    switch (state_) {
    case kOk:
      value_.~T();
      break;
    case kError:
      error_.~Error();
      break;
    }
    state_ = kError;
    new (&error_) Error(std::move(e));

    return *this;
  }

  Status &operator=(const Error &e) {
    switch (state_) {
    case kOk:
      value_.~T();
      break;
    case kError:
      error_.~Error();
      break;
    }
    state_ = kError;
    new (&error_) Error(e);

    return *this;
  }

  Status &operator=(T &&v) {
    switch (state_) {
    case kOk:
      value_.~T();
      break;
    case kError:
      error_.~Error();
      break;
    }
    state_ = kError;
    new (&value_) T(std::move(v));

    return *this;
  }

  Status(Status const &rhs) : state_(rhs.state_) {
    if (state_ == kOk) {
      value_ = rhs.value_;
    } else {
      error_ = rhs.error_;
    }
  }

  Status &operator=(Status const &rhs) {
    switch (state_) {
    case kOk:
      value_.~T();
      break;
    case kError:
      error_.~Error();
      break;
    }
    state_ = rhs.state_;
    switch (state_) {
    case kOk:
      new (&value_) T(rhs.value_);
      break;
    case kError:
      new (&error_) Error(rhs.error_);
      break;
    }
    state_ = rhs.state_;

    return *this;
  }

  Status &operator=(Status &&rhs) noexcept {
    switch (state_) {
    case kOk:
      value_.~T();
      break;
    case kError:
      error_.~Error();
      break;
    }
    switch (rhs.state_) {
    case kOk:
      new (&value_) T(std::move(rhs.value_));
      break;
    case kError:
      new (&error_) Error(std::move(rhs.error_));
      break;
    default: {
      state_ = kError;
      error_ = "rhs is moved";
      return *this;
    }
    }
    state_ = rhs.state_;
    rhs.state_ = kMoved;
    return *this;
  }

  bool ok() { return state_ == kOk; }

  explicit operator bool() const { return state_ == kOk; }

  void SetMoved() { state_ == kMoved; }
};

} // namespace toydb
