//
// Created by elf on 6/1/23.
//

#include "server/toydb_imp.h"
#include <absl/time/time.h>
using grpc::Server;
using grpc::Status;

namespace toydb {
    Status ToyDbServerImpl::Status(ServerContext *ctx, const StatusRequest *req,
                           StatusResponse *resp) {
        auto t = absl::Now();
        resp->set_id(id_);
        resp->set_time(ToUnixSeconds(t));
        return Status::OK;
    }

    Status ToyDbServerImpl::Get(ServerContext *ctx, const GetRequest *req, GetRequest *resp) {
        return Status::OK;
    }

    Status ToyDbServerImpl::Set(ServerContext *ctx, const SetRequest *req, SetResponse *resp) {
        return Status::OK;
    }
} // namespace toydb
