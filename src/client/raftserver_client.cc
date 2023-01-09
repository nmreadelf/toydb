//
// Created by elf on 9/1/23.
//

#include "raftserver_client.h"
using ::grpc::ClientContext;
using ::grpc::Status;

namespace {
Status RaftServerClientImpl::Step(::Message *msg) {
  ClientContext ctx;
  Success sucs;
  return stub_->Step(&ctx, *msg, &sucs);
}

} // namespace
