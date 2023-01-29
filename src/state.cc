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
  return OkWithValue(std::string(""));
}

Status<std::string> TestState::Mutate(const std::string &cmd) {
  cmds_.push_back(cmd);
  return OkWithValue(std::move(std::string(cmd)));
}

std::vector<std::string> TestState::List() { return cmds_; }
} // namespace toydb