//
// Created by kongsys on 15/1/23.
//

#ifndef MYTOYDB_STATE_H
#define MYTOYDB_STATE_H

#include "kv.h"

#include <absl/status/status.h>
#include <string>

using absl::Status;

namespace toydb {
class State {
public:
  virtual std::pair<Status, std::string> Mutate(const std::string &cmd) = 0;
};

class KvState : public State {
public:
  KvState() { kv_ = std::make_shared<KvStore>(); }

  std::pair<Status, std::string> Mutate(const std::string &cmd);

private:
  std::shared_ptr<KvStore> kv_;
};

class TestState : public State {
public:
  TestState() : cmds_() {}

  std::pair<Status, std::string> Mutate(const std::string &cmd);

  std::vector<std::string> List();

private:
  std::vector<std::string> cmds_;
};

} // namespace toydb

#endif // MYTOYDB_STATE_H
