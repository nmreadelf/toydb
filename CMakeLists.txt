cmake_minimum_required(VERSION 3.5.1)
project(MyToyDB C CXX)
find_path(PROTO_DEF "google/protobuf/port_def.inc")
find_program(_PROTOBUF_PROTOC "protoc")
find_program(_GRPC_CPP_PLUGIN_EXECUTABLE "grpc_cpp_plugin")
include_directories("${PROTO_DEF}")
find_package(gRPC CONFIG REQUIRED)
find_package(absl CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
#find_package(modules CONFIG REQUIRED)
#target_link_libraries(main PRIVATE gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts)
#target_link_libraries(main PRIVATE re2::re2 c-ares::cares)

# Generated sources
set(td_proto_srcs "${PROJECT_SOURCE_DIR}/src/proto/toydb.pb.cc")
set(td_proto_hdrs "${PROJECT_SOURCE_DIR}/src/proto/toydb.pb.h")
set(td_grpc_srcs "${PROJECT_SOURCE_DIR}/src/proto/toydb.grpc.pb.cc")
set(td_grpc_hdrs "${PROJECT_SOURCE_DIR}/src/proto/toydb.grpc.pb.h")

# Proto file
get_filename_component(td_proto "${PROJECT_SOURCE_DIR}/src/proto/toydb.proto" ABSOLUTE)
get_filename_component(td_proto_path "${td_proto}" PATH)

add_custom_command(
        OUTPUT "${td_proto_srcs}" "${td_proto_hdrs}" "${td_grpc_srcs}" "${td_grpc_hdrs}"
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${PROJECT_SOURCE_DIR}/src/proto/"
        --cpp_out "${PROJECT_SOURCE_DIR}/src/proto/"
        -I "${td_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${td_proto}"
        DEPENDS "${td_proto}")

# Include generated *.pb.h files
include_directories("${PROJECT_SOURCE_DIR}/src/proto/")
include_directories("${PROJECT_SOURCE_DIR}/src")

# hw_grpc_proto
add_library(td_grpc_proto "${td_proto_srcs}" "${td_proto_hdrs}" "${td_grpc_srcs}" "${td_grpc_hdrs}")
target_link_libraries(td_grpc_proto
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF})

add_library(kv "${PROJECT_SOURCE_DIR}/src/kv.cc" "${PROJECT_SOURCE_DIR}/src/kv.h")
target_link_libraries(kv absl::any)

add_executable(kv_test "${PROJECT_SOURCE_DIR}/src/kv_test.cc" )
target_link_libraries(kv_test kv GTest::gtest GTest::gtest_main absl::any absl::base absl::status)

add_executable(toydb "${PROJECT_SOURCE_DIR}/src/bin/toydb.cc" "${PROJECT_SOURCE_DIR}/src/server/toydb_imp.cc")
target_link_libraries(toydb td_grpc_proto
        gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts gRPC::grpc++_reflection
        absl::any absl::base absl::bits absl::city absl::flags absl::flags_parse absl::flags_config)
#target_link_libraries(server PRIVATE re2::re2 c-ares::cares)
