//
// Created by elf on 6/2/23.
//

#pragma once

#include <cstdint>
#include <cstdlib>
#include "status.h"

const uint64_t HEARTBEAT_INTERVAL  = 1;
const uint64_t ELECTION_TIMEOUT_MIN = 8 * HEARTBEAT_INTERVAL;
const uint64_t ELECTION_TIMEOUT_MAX = 15 * HEARTBEAT_INTERVAL;

namespace toydb::raft {
    class Candidate {
    public:
        Candidate(): election_ticks_(0), votes_(1) {
            election_timeout_ = std::rand() % (ELECTION_TIMEOUT_MAX - ELECTION_TIMEOUT_MIN);
        }
    private:
        // Transition to follower role.
        // follower error
        Status<uint64_t> BecomreFollower(uint64_t term, std::string& leader);

        // Transition to leader role.
        Status<uint64_t> BecomeLeader();
        uint64_t election_ticks_;
        uint64_t election_timeout_;
        uint64_t votes_;
    };

}