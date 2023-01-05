//
// Created by elf on 5/1/23.
//

#ifndef MYTOYDB_KV_H
#define MYTOYDB_KV_H
#include <absl/container/btree_map.h>
#include <string>
#include <cstdint>
#include <absl/status/status.h>



namespace toydb {
using ::absl::Status;
class Store {
public:
    Store(): data_() {
    }

    void Delete(const std::string& key);

    Status Get(const std::string& key, std::string *value);

    void Set(const std::string& key, const std::string &value);
private:
   absl::btree_map<std::string, std::string> data_;
};

}


#endif //MYTOYDB_KV_H
