//
// Created by elf on 6/1/23.
//

#ifndef MYTOYDB_TOYDB_IMPC_H
#define MYTOYDB_TOYDB_IMPC_H
#include "proto/toydb.grpc.pb.h"
#include <string>

using ::grpc::ServerContext;

namespace toydb {
    class ToyDbServerImpl final : public ToyDBServer::Service {
    public:
        ToyDbServerImpl(const std::string id) : id_(id){};

        ::grpc::Status Status(ServerContext *ctx, const StatusRequest *req,
                              StatusResponse *resp);

        ::grpc::Status Get(ServerContext *ctx, const GetRequest *req, GetRequest *resp);

        ::grpc::Status Set(ServerContext *ctx, const SetRequest *req, SetResponse *resp);

    private:
        std::string id_;
    };
} // namespace toydb


#endif //MYTOYDB_TOYDB_IMPC_H
