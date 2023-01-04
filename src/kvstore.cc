//
// Created by elf on 3/1/23.
//

#include "kvstore.h"
namespace toydb {
    Status kvstore::Get(std::string &key, std::string *value) {
        auto iter = data_.find(key);
        if (iter == data_.end()) {
            return ::absl::NotFoundError(key);
        }
        value->assign(iter->second.data(), iter->second.size());
        return Status();
    }
}