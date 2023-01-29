cmake_minimum_required(VERSION 3.5.1)
project(MyToyDB C CXX)
set(CMAKE_CXX_STANDARD 20)
find_path(PROTO_DEF "google/protobuf/port_def.inc")
find_program(_PROTOBUF_PROTOC "protoc")
find_program(_GRPC_CPP_PLUGIN_EXECUTABLE "grpc_cpp_plugin")
include_directories("${PROTO_DEF}")
find_package(gRPC CONFIG REQUIRED)
find_package(absl CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

# Generated proto sources
set(proto_files "")
function(generate_proto proto_name)
    get_filename_component(_proto "${PROJECT_SOURCE_DIR}/src/proto/${proto_name}.proto" ABSOLUTE)
    get_filename_component(_proto_path "${_proto}" PATH)
    add_custom_command(OUTPUT
            "${PROJECT_SOURCE_DIR}/src/proto/${proto_name}.pb.cc"
            "${PROJECT_SOURCE_DIR}/src/proto/${proto_name}.pb.h"
            "${PROJECT_SOURCE_DIR}/src/proto/${proto_name}.grpc.pb.cc"
            "${PROJECT_SOURCE_DIR}/src/proto/${proto_name}.grpc.pb.h"
            COMMAND ${_PROTOBUF_PROTOC}
            ARGS --grpc_out "${PROJECT_SOURCE_DIR}/src/proto/"
            --cpp_out "${PROJECT_SOURCE_DIR}/src/proto/"
            -I "${_proto_path}"
            --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
            "${_proto}"
            DEPENDS "${_proto}")
    list(APPEND proto_files "${PROJECT_SOURCE_DIR}/src/proto/${proto_name}.pb.cc"
            "${PROJECT_SOURCE_DIR}/src/proto/${proto_name}.grpc.pb.cc")
    set(proto_files ${proto_files} PARENT_SCOPE)
endfunction(generate_proto)
generate_proto("raft")
generate_proto("toydb")

# Include generated *.pb.h files
include_directories("${PROJECT_SOURCE_DIR}/src/proto/")
include_directories("${PROJECT_SOURCE_DIR}/src")

# hw_grpc_proto
add_library(toydb_grpc_proto "${proto_files}")
target_link_libraries(toydb_grpc_proto
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF})

add_library(kv "${PROJECT_SOURCE_DIR}/src/kv.cc" "${PROJECT_SOURCE_DIR}/src/kv.h" src/state.cc src/state.h src/status.h)
target_link_libraries(kv absl::any absl::status)

add_library(raft "${PROJECT_SOURCE_DIR}/src/raft/log.cc" src/status.h)
target_link_libraries(raft absl::any toydb_grpc_proto
        gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts gRPC::grpc++_reflection
        )

add_executable(kv_test "${PROJECT_SOURCE_DIR}/src/kv_test.cc" )
target_link_libraries(kv_test kv GTest::gtest GTest::gtest_main absl::any absl::base absl::status)

add_executable(raftlog_test "${PROJECT_SOURCE_DIR}/src/raft/log_test.cc")
target_link_libraries(raftlog_test absl::any absl::base absl::bits absl::city absl::status kv GTest::gtest GTest::gtest_main raft)

add_executable(toydb "${PROJECT_SOURCE_DIR}/src/bin/toydb.cc" "${PROJECT_SOURCE_DIR}/src/server/toydb_imp.cc" "${PROJECT_SOURCE_DIR}/src/server/raftserver_impl.cc" "${PROJECT_SOURCE_DIR}/src/client/raftserver_client.cc" src/status.h)
target_link_libraries(toydb toydb_grpc_proto
        gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts gRPC::grpc++_reflection
        absl::any absl::base absl::bits absl::city absl::flags absl::flags_parse absl::flags_config)
#target_link_libraries(server PRIVATE re2::re2 c-ares::cares)
