//
// Created by elf on 5/1/23.
//

#include "kv.h"
namespace toydb {
void KvStore::Delete(const std::string &key) { data_.erase(key); }

Status<std::string *> KvStore::Get(const std::string &key, std::string *value) {
  auto iter = data_.find(key);
  if (iter == data_.end()) {
    return Status<std::string *>(std::move(key + " not found"));
  }
  value->assign(iter->second.data(), iter->second.size());
  return OkWithValue(value);
}

void KvStore::Set(const std::string &key, const std::string &value) {
  data_[key] = value;
}
} // namespace toydb