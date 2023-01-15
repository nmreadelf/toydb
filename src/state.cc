//
// Created by kongsys on 15/1/23.
//

#include "state.h"

#include "proto/raft.pb.h"

using rState = raft::State;

namespace toydb {
std::pair<absl::Status, std::string> KvState::Mutate(const std::string &cmd) {
  rState s;
  s.ParseFromString(cmd);
  kv_->Set(s.key(), s.value());
  return std::make_pair(absl::OkStatus(), "");
}

std::pair<Status, std::string> TestState::Mutate(const std::string &cmd) {
  cmds_.push_back(cmd);
  return std::make_pair(absl::OkStatus(), cmd);
}

std::vector<std::string> TestState::List() { return cmds_; }
} // namespace toydb