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
  if (options_.log == nullptr) {
  }
  return {true};
}
} // namespace toydb::raft