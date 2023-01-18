//
// Created by elf on 9/1/23.
//

#include "log.h"
#include <cstdint>
#include <ranges>

namespace toydb::raft {
std::pair<Status, Log *> Log::Build(std::shared_ptr<toydb::KvStore> s) {
  static int n = sizeof(uint64_t);
  uint64_t last_index = 0;
  uint64_t last_term = 0;
  uint64_t apply_index = 0;
  uint64_t apply_term = 0;
  uint64_t commit_index = 0;
  uint64_t commit_term = 0;
  std::string v;
  v.resize(n);
  // decode uint64_t
  auto ok = s->Get("apply_index", &v);
  {
    char *rbuffer = reinterpret_cast<char *>(&(v[0]));
    if (ok.ok()) {
      memcpy(static_cast<void *>(&apply_index), rbuffer, n);
    }
  }

  ok = s->Get(std::to_string(apply_index), &v);
  if (ok.ok()) {
    Entry e;
    bool st = e.ParseFromString(v);
    if (!st) {
      return std::make_pair(absl::AbortedError("parse entry " +
                                               std::to_string(apply_index) +
                                               " fail"),
                            nullptr);
    }
    commit_term = e.term();
  } else if (apply_index != 0) {
    return std::make_pair(absl::AbortedError("Applied entry " +
                                             std::to_string(apply_index) +
                                             " not found"),
                          nullptr);
  }
  commit_index = apply_index;

  apply_term = commit_term;
  for (uint64_t i :
       std::ranges::iota_view(uint64_t(1)) | std::views::take(UINT64_MAX)) {
    ok = s->Get(std::to_string(i), &v);
    if (!ok.ok()) {
      break;
    }
    Entry e;
    bool st = e.ParseFromString(v);
    if (!st) {
      return std::make_pair(
          absl::AbortedError("parse entry " + std::to_string(i) + " fail"),
          nullptr);
    }
    last_index = i;
    last_term = e.term();
  }
  auto log = new Log(s, last_index, last_term, commit_index, commit_term,
                     apply_index, apply_term);
  /*
   * encode uint64_t
   * std::string v;
   * int n = sizeof(last_index_);
   * v.resize(n);
   * char* buf = reinterpret_cast<char*>(&(v[0]));
   * memcpy(rbuffer, static_cast<void*>(&last_index_), n)
   */

  return std::make_pair(absl::OkStatus(), log);
}

std::pair<Status, uint64_t> Log::Append(Entry &entry) {
  last_index_++;
  last_term_ = entry.term();
  kv_->Set(std::to_string(last_index_), entry.SerializeAsString());
  return std::make_pair(absl::OkStatus(), last_index_);
}

std::tuple<Status, uint64_t, std::string> Log::Apply(State *state) {
  if (apply_index_ >= commit_index_) {
    return std::make_tuple(absl::OkStatus(), 0, "");
  }

  std::string output;
  auto res = Get(apply_index_ + 1);
  if (res.first.ok()) {
    apply_index_++;
    apply_term_ = res.second->term();
    if (!res.second->command().empty()) {
      auto r = state->Mutate(res.second->command());
      output = r.second;
    }
  }

  std::string v;
  int n = sizeof(apply_index_);
  v.resize(n);
  char *buf = reinterpret_cast<char *>(&(v[0]));
  memcpy(buf, static_cast<void *>(&apply_index_), n);
  kv_->Set("apply_index", v);
  return std::make_tuple(absl::OkStatus(), apply_index_, std::move(output));
}

std::tuple<Status, uint64_t> Log::Commit(uint64_t index) {
  index = std::min(index, last_index_);
  index = std::max(index, commit_index_);
  if (index != commit_index_) {
    auto res = Get(index);
    if (!res.first.ok()) {
      return std::make_tuple(absl::NotFoundError("Entry at commit index " +
                                                 std::to_string(index) +
                                                 " does not exist"),
                             index);
    }
    commit_index_ = index;
    commit_term_ = res.second->term();
  }

  return std::make_tuple(absl::OkStatus(), index);
}

std::pair<Status, std::shared_ptr<Entry>> Log::Get(uint64_t index) {
  std::string v;
  auto ok = kv_->Get(std::to_string(index), &v);
  if (!ok.ok()) {
    return std::make_pair(
        absl::NotFoundError("not found index " + std::to_string(index)),
        nullptr);
  }
  auto entry = new Entry();
  entry->ParseFromString(v);
  return std::make_pair(absl::OkStatus(), std::shared_ptr<Entry>(entry));
}

bool Log::Has(uint64_t index, uint64_t term) {
  if (index == 0 && term == 0) {
    return true;
  }

  auto res = Get(index);
  if (res.first.ok()) {
    return res.second->term() == term;
  }
  return false;
}
std::shared_ptr<std::vector<std::shared_ptr<Entry>>>
Log::Range(uint64_t start) {
  auto es = std::shared_ptr<std::vector<std::shared_ptr<Entry>>>();
  for (uint64_t i : std::ranges::iota_view{start, last_index_ + 1}) {
    auto res = Get(i);
    if (res.first.ok()) {
      es->push_back(std::shared_ptr<Entry>(res.second));
    }
  }
  return es;
}

std::pair<Status, uint64_t> Log::Splice(uint64_t base, uint64_t base_term,
                                        std::vector<Entry *> &entrys) {
  if (base > 0 && !Has(base, base_term)) {
    return std::make_pair(
        absl::NotFoundError("raft base " + std::to_string(base) + ":" +
                            std::to_string(base_term) + " not found"),
        0);
  }

  uint64_t i = 0;
  for (int i = 0; i < entrys.size(); i++) {
    const auto &e = entrys[i];
    auto res = Get(base + i + 1);
    if (!res.first.ok()) {
      Append(*e);
      continue;
    }
    if (res.second->term() == e->term()) {
      continue;
    }
    auto ok = Truncate(base + i);
    if (!ok.first.ok()) {
      return ok;
    }
  }
  return std::make_pair(absl::OkStatus(), last_index_);
}

std::pair<Status, uint64_t> Log::Truncate(uint64_t index) {
  if (index < apply_index_) {
    return std::make_pair(absl::AbortedError("cannot remove applied log entry"),
                          0);
  }
  if (index < commit_index_) {
    return std::make_pair(
        absl::AbortedError("cannot remove committed log entry"), 0);
  }

  for (uint64_t i : std::ranges::iota_view{index + 1, last_index_ + 1}) {
    kv_->Delete(std::to_string(i));
  }
  last_index_ = std::min(index, last_index_);
  auto res = Get(last_index_);
  if (!res.first.ok()) {
    return std::make_pair(res.first, 0);
  }
  last_term_ = res.second->term();
  return std::make_pair(absl::OkStatus(), last_index_);
}

std::tuple<Status, uint64_t, std::string> Log::LoadTerm() {
  std::string v;
  auto ok = kv_->Get("term", &v);
  if (!ok.ok()) {
    return std::make_tuple(absl::OkStatus(), 0, "");
  }
  uint64_t term = 0;
  int n = sizeof(term);
  v.resize(n);
  char *buf = reinterpret_cast<char *>(&(v[0]));
  memcpy(static_cast<void *>(&term), buf, n);
  ok = kv_->Get("voted_for", &v);
  if (!ok.ok()) {
    return std::make_tuple(absl::OkStatus(), term, "");
  }
  return std::make_tuple(ok, term, v);
}

Status Log::SaveTerm(uint64_t term, std::string &vote_for) {
  if (term > 0) {
    std::string v;
    int n = sizeof(term);
    v.resize(n);
    char *buf = reinterpret_cast<char *>(&(v[0]));
    memcpy(buf, static_cast<void *>(&term), n);
    kv_->Set("term", v);
  } else {
    kv_->Delete("term");
  }
  if (vote_for.empty()) {
    kv_->Delete("voted_for");
  } else {
    kv_->Set("voted_for", vote_for);
  }
  return absl::OkStatus();
}

} // namespace toydb::raft