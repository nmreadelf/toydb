//
// Created by elf on 5/1/23.
//

#include "kv.h"
namespace toydb {
void KvStore::Delete(const std::string &key) { data_.erase(key); }

Status KvStore::Get(const std::string &key, std::string *value) {
  auto iter = data_.find(key);
  if (iter == data_.end()) {
    return absl::NotFoundError(key);
  }
  value->assign(iter->second.data(), iter->second.size());
  return absl::OkStatus();
}

void KvStore::Set(const std::string &key, const std::string &value) {
  data_[key] = value;
}
} // namespace toydb