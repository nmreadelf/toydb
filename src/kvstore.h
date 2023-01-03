//
// Created by elf on 3/1/23.
//

#ifndef MYTOYDB_KVSTORE_H
#define MYTOYDB_KVSTORE_H
#include "status.h"

class kvstore {
public:
  kvstore() : _data() {}

  ~kvstore() {}

  void delete (std::string &key) { data_.erase(key); }

  Status get(std::string &key, std::string *value) {
    auto iter = data_.find(key);
    if (iter == data_.end()) {
      return Status::NotFound();
    }
    value->assign(iter->second.data(), iter->second.size());
    return Status();
  }

  void put(std::string &key, std::string : value) { data_[key] = value; }

private:
  std::map<std::string, std::string> data_;
};

#endif // MYTOYDB_KVSTORE_H
