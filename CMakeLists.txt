cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

# set(Protobuf_PREFIX_PATH
#     "F:/Protobuf/build-debug/Protobuf/include"            
#     "F:/Protobuf/build-debug/Protobuf/lib"             
#     "F:/Protobuf/build-debug/Protobuf/bin" )
# list(APPEND CMAKE_PREFIX_PATH "${Protobuf_PREFIX_PATH}")

set(CMAKE_PREFIX_PATH "F:/Protobuf/build-debug/Protobuf")

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

include_directories("F:/Protobuf/build-debug/Protobuf/include")

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto map_renderer.proto svg.proto graph.proto transport_router.proto)

set(TC_FILES domain.cpp domain.h geo.cpp geo.h graph.h json.cpp json.h json_builder.cpp json_builder.h json_reader.cpp json_reader.h main.cpp map_renderer.cpp map_renderer.h ranges.h request_handler.cpp request_handler.h router.h serialization.cpp serialization.h svg.cpp svg.h transport_catalogue.cpp transport_catalogue.h transport_router.cpp transport_router.h transport_catalogue.proto map_renderer.proto svg.proto graph.proto transport_router.proto)

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${TC_FILES})
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)