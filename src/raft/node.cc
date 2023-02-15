//
// Created by elf on 13/2/23.
//

#include "raft/node.h"

namespace toydb::raft {
Status<bool> Node::BecomeCandidate() {
  // Starting election for term {term_ + 1}
  return {true};
}

Node::Node(std::string &id) : id_(id), term_(0) {}

Status<bool> Node::Init(const NodeOption &options) {
  options_ = options;
  leader_seen_timeout_ = options_.election_timeout;
  log_ = options_.log;
  if (auto res = InitLog(); !res) {
    return {res.error_};
  }
  role_ = RoleState::ROLE_STATE_FOLLOWER;
  return {true};
}

Status<bool> Node::InitLog() {
  if (options_.log != nullptr) {

    log_ = options_.log;
  } else {
    std::shared_ptr<KvStore> kv = std::make_shared<KvStore>();
    auto res = Log::Build(kv);
    if (!res) {
      return {res.error_};
    }
    log_ = res.value_;
  }
  return {true};
}
} // namespace toydb::raft