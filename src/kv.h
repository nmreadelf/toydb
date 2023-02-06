//
// Created by elf on 5/1/23.
//

#pragma once
#include "status.h"
#include <absl/container/btree_map.h>
#include <cstdint>
#include <string>

namespace toydb {
class KvStore {
public:
  KvStore() : data_() {}

  void Delete(const std::string &key);

  bool Get(const std::string &key, std::string *value);

  void Set(const std::string &key, const std::string &value);

private:
  absl::btree_map<std::string, std::string> data_;
};

} // namespace toydb
