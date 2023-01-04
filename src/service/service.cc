//
// Created by elf on 4/1/23.
//

#include "service.h"
#include <absl/time/time.h>
using grpc::Server;
using grpc::Status;

namespace toydb {
    Status service::Status(ServerContext *ctx, const StatusRequest *req, StatusResponse *resp) {
        auto t = absl::Now();
        resp->set_id(id_);
        resp->set_time(ToUnixSeconds(t));
        return Status::OK;
    }
}