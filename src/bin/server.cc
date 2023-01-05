//
// Created by elf on 4/1/23.
//

#include <absl/flags/declare.h>
#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/xds_server_builder.h>

#include <iostream>

#include "service/service.h"

ABSL_FLAG(std::string, addr, "0.0.0.0:50051", "Server address for service.");
ABSL_FLAG(std::string, id, "", "Server id for service.");

void RunServer() {
  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  std::string addr = absl::GetFlag(FLAGS_addr);
  std::string id = absl::GetFlag(FLAGS_id);
  if (addr.empty()) {
    std::cerr << "must provide addr flag" << std::endl;
    return;
  }
  if (id.empty()) {
    id = addr;
    std::cout << "use addr flag as id flag" << std::endl;
  }
  toydb::service service(id);
  // grpc::XdsServerBuilder xds_builder;
  // xds_builder.RegisterService(&service);
  grpc::ServerBuilder builder;
  builder.RegisterService(&service);
  builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
  // auto server = xds_builder.BuildAndStart();
  auto server = builder.BuildAndStart();
  std::cout << "Server listening on " << addr << std::endl;
  server->Wait();
}

int main(int argc, char *argv[]) {
  // Overrides the default for FLAGS_logtostderr
  // If the command-line contains a value for logtostderr, use that. Otherwise,
  // use the default (as set above).
  absl::ParseCommandLine(argc, argv);
  RunServer();
  return 0;
}