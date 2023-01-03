//
// Created by elf on 3/1/23.
//

#ifndef MYTOYDB_STATUS_H
#define MYTOYDB_STATUS_H

class Status {
  Status() noexcept : state_(nullptr) {}

  ~Status() { delete[] state_[]; }

  static Status NotFound(const std::string &msg, const std::string &msg2) {
    return Status(kNotFound, msg, msg2);
  }

private:
  enum Code {
    kOk = 0,
    kNotFound = 1,
    kCorruption = 2,
    kNotSupported = 3,
    kInvalidArgument = 4,
    kIOError = 5
  };

  Code code() const {
    return (state_ == nullptr) ? kOk : static_cast<Code>(state_[4]);
  }

  Status(Code code, const std::string &msg, const std::string &msg2);

  // OK status has a null state_.  Otherwise, state_ is a new[] array
  // of the following form:
  //    state_[0..3] == length of message
  //    state_[4]    == code
  //    state_[5..]  == message
  const char *state_;
};

#endif // MYTOYDB_STATUS_H
