//
// Created by kongsys on 15/1/23.
//

#pragma once
#include "kv.h"
#include "status.h"

#include <string>

namespace toydb {
class State {
public:
  virtual Status<std::string> Mutate(const std::string &cmd) = 0;

  virtual Status<std::string> Read(const std::string &cmd) = 0;
};

class KvState : public State {
public:
  KvState() { kv_ = std::make_shared<KvStore>(); }

  Status<std::string> Mutate(const std::string &cmd) override;

private:
  std::shared_ptr<KvStore> kv_;
};

class TestState : public State {
public:
  TestState() : cmds_() {}

  Status<std::string> Mutate(const std::string &cmd) override;

  Status<std::string> Read(const std::string &cmd);

  std::vector<std::string> List();

private:
  std::vector<std::string> cmds_;
};

} // namespace toydb
