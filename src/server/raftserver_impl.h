//
// Created by elf on 9/1/23.
//

#ifndef MYTOYDB_RAFTSERVER_IMPL_H
#define MYTOYDB_RAFTSERVER_IMPL_H
#include "client/raftserver_client.h"
#include "proto/raft.grpc.pb.h"
#include <map>
#include <string>

using ::grpc::ServerContext;
using ::raft::Message;
using ::raft::RaftServer;
using ::raft::Success;

namespace toydb {
class RaftServerImpl final : public RaftServer::Service {
public:
  explicit RaftServerImpl(const std::map<std::string, std::string> addresses)
      : peers_() {
    for (const auto &[name, addr] : addresses) {
      peers_.insert_or_assign(name, RaftServerClientImpl(addr));
    }
  };

  RaftServerImpl() : peers_() {}

  ::grpc::Status Send(::raft::Message *msg);

  // below is grpc method

  ::grpc::Status Step(ServerContext *ctx, const Message *req, Success *resp);

private:
  std::map<std::string, RaftServerClientImpl> peers_;
};
} // namespace toydb

#endif // MYTOYDB_RAFTSERVER_IMPL_H
