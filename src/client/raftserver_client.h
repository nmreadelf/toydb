//
// Created by elf on 9/1/23.
//

#ifndef MYTOYDB_RAFTSERVER_CLIENT_H
#define MYTOYDB_RAFTSERVER_CLIENT_H
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
    stub_ = RaftServer::NewStub(channel_);
  };

  Status Step(::Message *msg);

private:
  std::string addr_;
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<RaftServer::Stub> stub_;
};

} // namespace

#endif // MYTOYDB_RAFTSERVER_CLIENT_H
