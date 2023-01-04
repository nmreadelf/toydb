//
// Created by elf on 3/1/23.
//

#ifndef MYTOYDB_KVSTORE_H
#define MYTOYDB_KVSTORE_H
#include <map>
#include <absl/status/status.h>

using ::absl::Status;

namespace toydb {
    class kvstore {
    public:
        kvstore() : data_() {}

        ~kvstore() {}

        void Delete(std::string &key) { data_.erase(key); }

        Status Get(std::string &key, std::string *value);

        void Put(std::string &key, std::string* value) { data_[key] = value; }

    private:
        std::map<std::string, std::string> data_;
    };
}

#endif // MYTOYDB_KVSTORE_H
