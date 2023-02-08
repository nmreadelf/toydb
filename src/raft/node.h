//
// Created by elf on 8/2/23.
//

#pragma once
#include "status.h"
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <variant>
#include <vector>
// The interval between leader heartbeats, in ticks.
const uint64_t HEARTBEAT_INTERVAL = 1;

// The minimum election timeout, in ticks.
const uint64_t ELECTION_TIMEOUT_MIN = 8 * HEARTBEAT_INTERVAL;

// The maximum election timeout, in ticks.
const uint64_t ELECTION_TIMEOUT_MAX = 15 * HEARTBEAT_INTERVAL;

namespace toydb::raft {
enum RoleState {
  ROLE_STATE_LEADER = 1,
  ROLE_STATE_CANDIDATE = 2,
  ROLE_STATE_FOLLOWER = 3,
};

class Node {
public:
  Node(std::string &leader, std::string &vote_for)
      : leader_(leader), voted_for_(vote_for), leader_seen_ticks_(0),
        proxy_calls_() {
    leader_seen_timeout_ =
        std::rand() % (ELECTION_TIMEOUT_MAX - ELECTION_TIMEOUT_MIN);
  }
  Node() : heartbeat_ticks_(0), votes_(1) {
    election_timeout_ =
        std::rand() % (ELECTION_TIMEOUT_MAX - ELECTION_TIMEOUT_MIN);
  }

  Node(std::vector<std::string> &peers, uint64_t last_index_)
      : heartbeat_ticks_(0), peer_next_index_(), peer_last_index_(), calls_() {
    for (const auto &p : peers) {
      peer_next_index_[p] = last_index_ + 1;
      peer_last_index_[p] = 0;
    }
  }

private:
  // Transforms the node into a candidate.
  Status<bool> BecomeCandidate();

  // Checks if the message sender is the current leader
  bool IsLeader(std::string &from);

  // Processes a message
  Status<bool> Step(std::string &msg);

  Status<bool> Tick();

  // The leader, or empty if just initialized,
  std::string leader_;
  // The number of ticks since the last message from the leader.
  uint64_t leader_seen_ticks_;
  // The timeout before triggering an election.
  uint64_t leader_seen_timeout_;
  // The node we voted for in the current term, if any.
  std::string voted_for_;
  // Keeps track of any proxied calls to the leader (call ID to message sender).
  std::map<std::string, std::string> proxy_calls_;

  // for candidate
private:
  // Transition to follower role.
  // follower error
  Status<bool> BecomeFollower(uint64_t term, std::string &leader);

  // Transition to leader role.
  Status<bool> BecomeLeader();

  // // Processes a message.
  // Status<bool> Step(std::string &msg);

  // // Processes a logical clock tick.
  // Status<bool> Tick();

  uint64_t election_ticks_;
  uint64_t election_timeout_;
  uint64_t votes_;

  // for leader
public:
  // A mutation submitted to the Raft log.
  struct MutateState {
    uint64_t log_index_;
  };

  struct ReadState {
    std::string cmd_;
    uint64_t commit_index_;
    uint64_t quorum_;
    std::set<std::string> votes_;
  };
  struct Call {
    std::string id_;
    std::string from_;
    std::variant<MutateState, ReadState> operation_;
  };

private:
  // Number of ticks since last heartbeat.
  uint64_t heartbeat_ticks_;
  // The next index to replicate to a peer.
  std::map<std::string, uint64_t> peer_next_index_;
  // The last index known to be replicated on a peer.
  std::map<std::string, uint64_t> peer_last_index_;
  // Any client calls being processed.
  std::vector<Call> calls_;
};
};
}
