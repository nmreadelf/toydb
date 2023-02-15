//
// Created by kongsys on 15/1/23.
//

#include "state.h"
#include "status.h"

#include "proto/raft.pb.h"

using rState = raft::State;

namespace toydb {

Status<std::string> KvState::Mutate(const std::string &cmd) {
  rState s;
  s.ParseFromString(cmd);
  kv_->Set(s.key(), s.value());
  return {std::string("")};
}

Status<std::string> TestState::Mutate(const std::string &cmd) {
  cmds_.push_back(cmd);
  return {cmd};
}

Status<std::string> TestState::Read(const std::string &cmd) {
  if (cmd.size() != 1) {
    return {Error("Read payload must be 1 byte")};
  }
  int32_t idx = uint32_t(cmd[0]);
  if (idx >= cmds_.size()) {
    return {"0"};
  }
  return {cmds_[idx]};
}

std::vector<std::string> TestState::List() { return cmds_; }
} // namespace toydb