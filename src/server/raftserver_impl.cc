//
// Created by elf on 9/1/23.
//

#include "raftserver_impl.h"
#include <absl/strings/str_format.h>

using ::grpc::Status;

namespace toydb {
::grpc::Status RaftServerImpl::Send(::Message *msg) {
  auto iter = peers_.find(msg->to());
  if (iter == peers_.end()) {
    return grpc::Status(grpc::NOT_FOUND, "Unkown Raft peer " + msg->to());
  }
  return iter->second.Step(msg);
}

Status RaftServerImpl::Step(ServerContext *ctx, const Message *req,
                            Success *resp) {
  return Status::OK;
}
} // namespace toydb
