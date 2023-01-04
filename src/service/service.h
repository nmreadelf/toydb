//
// Created by elf on 4/1/23.
//

#ifndef MYTOYDB_SERVICE_H
#define MYTOYDB_SERVICE_H
#include <string>
#include "proto/toydb.grpc.pb.h"

using ::grpc::ServerContext;

namespace toydb {
class service final : public ToyDB::Service {
    public:
        service(const std::string id): id_(id) {};

    ::grpc::Status Status(ServerContext* ctx, const StatusRequest* req, StatusResponse* resp);

    private:
        std::string id_;
    };
}

#endif //MYTOYDB_SERVICE_H
