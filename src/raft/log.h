//
// Created by elf on 9/1/23.
//

#pragma once
#include "kv.h"
#include "proto/raft.pb.h"
#include "state.h"
#include <absl/status/status.h>
#include <memory>
#include <utility>

using raft::Entry;

namespace toydb::raft {
class Log {
public:
  Log(const Log &) = delete;

  Log &operator=(const Log &) = delete;

  // Creates a new Log, using a KvStore for storage.
  static Status<Log *> Build(std::shared_ptr<toydb::KvStore> s);

  // Appends an entry to the Log.
  Status<uint64_t> Append(Entry &entry);

  // Applies the next commited entry to the state machine, if any.
  // Returns the applied entry index and output, or None if no entry.
  Status<std::tuple<uint64_t, std::string>> Apply(State *state);

  // Comits entries ujp to and including an index
  Status<uint64_t> Commit(uint64_t index);

  // Fetches an entry at an index
  Status<Entry *> Get(uint64_t index);

  // Fetches the last applied index and term
  std::pair<uint64_t, uint64_t> GetApplied() {
    return std::make_pair(apply_index_, apply_term_);
  }

  // Fetches the last committed index and term
  std::pair<uint64_t, uint64_t> GetCommitted() {
    return std::make_pair(commit_index_, commit_term_);
  }

  // Fetches the last stored index and term
  std::pair<uint64_t, uint64_t> GetLast() {
    return std::make_pair(last_index_, last_term_);
  }

  // Checks if the log containes and entry
  bool Has(uint64_t index, uint64_t term);

  // Fetches a range of entries
  std::shared_ptr<std::vector<std::shared_ptr<Entry>>> Range(uint64_t start);

  // Splices a set of entries onto an offset. The semantics are a bit unusual,
  Status<uint64_t> Splice(uint64_t base, uint64_t base_term,
                          std::vector<Entry *> &entrys);

  // Truncates the log such that its last item is at most index.
  // Refuses to remove entries that have been applied or committed.
  Status<uint64_t> Truncate(uint64_t index);

  // Loads information about the most recent term known by the log,
  // containing the term number (0 if none) and candidate voted for
  // in current term (if any).
  Status<std::tuple<uint64_t, std::string>> LoadTerm();

  // Saves information about the most recent term.
  Status<std::nullptr_t> SaveTerm(uint64_t term, std::string &vote_for);

private:
  explicit Log(std::shared_ptr<toydb::KvStore> s, uint64_t last_index,
               uint64_t last_term, uint64_t commit_index, uint64_t commit_term,
               uint64_t apply_index, uint64_t apply_term)
      : kv_(std::move(s)), last_index_(last_index), last_term_(last_term),
        commit_index_(commit_index), commit_term_(commit_term),
        apply_index_(apply_index), apply_term_(apply_term) {}
  // The underlying key-value store.
  std::shared_ptr<toydb::KvStore> kv_;
  // The index of the last stored entry.
  uint64_t last_index_;
  // the term of the last stored entry.
  uint64_t last_term_;
  // The last entry known to be commited. Not persistend,
  // since leaders will determine this when they're elected.
  uint64_t commit_index_;
  // The term of the last commited entry.
  uint64_t commit_term_;
  // The last entry appied to the state machine. This is
  // presisted, since the state machine is also presistend.
  uint64_t apply_index_;
  // The term of the last applied entry.
  uint64_t apply_term_;
};

} // namespace toydb::raft
