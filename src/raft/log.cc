//
// Created by elf on 9/1/23.
//

#include "log.h"
#include <cstdint>
#include <queue>
#include <ranges>

namespace toydb::raft {
Status<Log *> Log::Build(std::shared_ptr<toydb::KvStore> s) {
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
  {
    auto ok = s->Get("apply_index", &v);
    char *rbuffer = reinterpret_cast<char *>(&(v[0]));
    if (ok) {
      memcpy(static_cast<void *>(&apply_index), rbuffer, n);
    }
  }

  {
    auto ok = s->Get(std::to_string(apply_index), &v);
    if (ok) {
      Entry e;
      bool st = e.ParseFromString(v);
      if (!st) {
        return {
            Error("parse apply idx " + std::to_string(apply_index) + " fail")};
      }
      commit_term = e.term();
    } else if (apply_index != 0) {
      return {
          Error("Applied entry " + std::to_string(apply_index) + " not found")};
    }
  }
  commit_index = apply_index;

  apply_term = commit_term;
  for (uint64_t i :
       std::ranges::iota_view(uint64_t(1)) | std::views::take(UINT64_MAX)) {
    auto ok = s->Get(std::to_string(i), &v);
    if (!ok) {
      break;
    }
    Entry e;
    bool st = e.ParseFromString(v);
    if (!st) {
      return {Error("parse entry " + std::to_string(apply_index) + " fail")};
    }
    last_index = i;
    last_term = e.term();
  }
  Log *log = new Log(s, last_index, last_term, commit_index, commit_term,
                     apply_index, apply_term);
  /*
   * encode uint64_t
   * std::string v;
   * int n = sizeof(last_index_);
   * v.resize(n);
   * char* buf = reinterpret_cast<char*>(&(v[0]));
   * memcpy(rbuffer, static_cast<void*>(&last_index_), n)
   */

  return {log};
}

Status<uint64_t> Log::Append(Entry &entry) {
  last_index_++;
  last_term_ = entry.term();
  kv_->Set(std::to_string(last_index_), entry.SerializeAsString());
  return {last_index_};
}

Status<std::tuple<uint64_t, std::string>> Log::Apply(State *state) {
  if (apply_index_ >= commit_index_) {
    return {std::make_tuple(uint64_t(0), std::string())};
  }

  std::string output;
  if (auto res = Get(apply_index_ + 1); res) {
    apply_index_++;
    apply_term_ = res.value_->term();
    if (!res.value_->command().empty()) {
      auto r = state->Mutate(res.value_->command());
      output = std::move(r.value_);
    }
  }

  std::string v;
  int n = sizeof(apply_index_);
  v.resize(n);
  memcpy(v.data(), static_cast<void *>(&apply_index_), n);
  kv_->Set("apply_index", v);
  return {std::make_tuple(apply_index_, std::move(output))};
}

Status<uint64_t> Log::Commit(uint64_t index) {
  index = std::min(index, last_index_);
  index = std::max(index, commit_index_);
  if (index != commit_index_) {
    auto res = Get(index);
    if (!res) {
      return {Error("Entry at commit index " + std::to_string(index) +
                    " does not exist")};
    }
    commit_index_ = index;
    commit_term_ = res.value_->term();
    std::shared_ptr<Entry> st(res.value_);
  }

  return {index};
}

Status<Entry *> Log::Get(uint64_t index) {
  std::string v;
  auto ok = kv_->Get(std::to_string(index), &v);
  if (!ok) {
    return {Error(std::to_string(index) + " idx not found")};
  }
  auto entry = new Entry();
  entry->ParseFromString(v);
  return {entry};
}

bool Log::Has(uint64_t index, uint64_t term) {
  if (index == 0 && term == 0) {
    return true;
  }

  if (auto res = Get(index); res) {
    return res.value_->term() == term;
  }
  return false;
}
std::shared_ptr<std::vector<std::shared_ptr<Entry>>>
Log::Range(uint64_t start) {
  auto es = std::make_shared<std::vector<std::shared_ptr<Entry>>>();
  for (uint64_t i : std::ranges::iota_view{start, last_index_ + 1}) {
    if (auto res = Get(i); res) {
      es->push_back(std::shared_ptr<Entry>(res.value_));
    }
  }
  return es;
}

Status<uint64_t> Log::Splice(uint64_t base, uint64_t base_term,
                             std::vector<Entry *> &entrys) {
  if (base > 0 && !Has(base, base_term)) {
    return {Error("raft base " + std::to_string(base) + ":" +
                  std::to_string(base_term) + " not found")};
  }

  for (int i = 0; i < entrys.size(); i++) {
    const auto &e = entrys[i];
    if (auto res = Get(base + i + 1); res) {
      if (res.value_->term() == e->term()) {
        continue;
      }
      if (auto ok = Truncate(base + i); !ok) {
        return ok;
      }
    }
    Append(*e); // ignore Append result
  }
  return {last_index_};
}

Status<uint64_t> Log::Truncate(uint64_t index) {
  if (index < apply_index_) {
    // return Status<uint64_t>("cannot remove applied log entry");
    return {Error("cannot remove applied log entry")};
  }
  if (index < commit_index_) {
    // return Status<uint64_t>("cannot remove committed log entry");
    return {Error("cannot remove committed log entry")};
  }

  for (uint64_t i = index + 1; i <= last_index_; i++) {
    kv_->Delete(std::to_string(i));
  }
  last_index_ = std::min(index, last_index_);
  auto res = Get(last_index_);
  if (!res) {
    return {uint64_t(0)};
  }
  last_term_ = res.value_->term();
  return {last_index_};
}

Status<std::tuple<uint64_t, std::string>> Log::LoadTerm() {
  std::string v;
  auto ok = kv_->Get("term", &v);
  if (!ok) {
    return {std::make_tuple(uint64_t(0), std::string())};
  }
  uint64_t term = 0;
  int n = sizeof(term);
  memcpy(static_cast<void *>(&term), v.data(), n);
  ok = kv_->Get("voted_for", &v);
  if (!ok) {
    return {std::make_tuple(term, std::string())};
  }
  return {std::make_tuple(term, v)};
}

Status<nullptr_t> Log::SaveTerm(uint64_t term, std::string &vote_for) {
  if (term > 0) {
    std::string v;
    int n = sizeof(term);
    v.resize(n);
    memcpy(v.data(), static_cast<void *>(&term), n);
    kv_->Set("term", v);
  } else {
    kv_->Delete("term");
  }
  if (vote_for.empty()) {
    kv_->Delete("voted_for");
  } else {
    kv_->Set("voted_for", vote_for);
  }
  return {nullptr};
}

} // namespace toydb::raft