//
// Created by elf on 9/1/23.
//

#pragma once
#include "proto/raft.grpc.pb.h"
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/grpcpp.h>
#include <memory>

using ::grpc::Status;

namespace {
class RaftServerClientImpl {
public:
  RaftServerClientImpl(const std::string &addr) : addr_(addr) {
    channel_ = grpc::CreateChannel(addr_, grpc::InsecureChannelCredentials());
    stub_ = raft::RaftServer::NewStub(channel_);
  };

  Status Step(raft::Message *msg);

private:
  std::string addr_;
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<raft::RaftServer::Stub> stub_;
};

} // namespace
