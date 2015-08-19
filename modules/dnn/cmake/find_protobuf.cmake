if(NOT BUILD_LIBPROTOBUF_FROM_SOURCES)
  find_package(Protobuf)
endif()

if(NOT BUILD_LIBPROTOBUF_FROM_SOURCES AND PROTOBUF_FOUND AND EXISTS ${PROTOBUF_PROTOC_EXECUTABLE})
  message(STATUS "The protocol buffer compiler and libprotobuf were found")
  PROTOBUF_GENERATE_CPP(PROTOBUF_HDRS PROTOBUF_SRCS src/caffe/caffe.proto)
  add_definitions(-DHAVE_PROTOBUF=1)
else()
  if(NOT PROTOBUF_FOUND)
    message(STATUS "libprotobuf not found")
  else()
    message(STATUS "The protocol buffer compiler not found")
  endif()

  if(NOT OPENCV_INITIAL_PASS)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/protobuf)
  endif()

  set(PROTOBUF_LIBRARIES libprotobuf)
  set(PROTOBUF_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/protobuf/src ${CMAKE_CURRENT_BINARY_DIR})
  set(PROTOBUF_SRCS ${CMAKE_CURRENT_BINARY_DIR}/caffe.pb.cc)
  set(PROTOBUF_HDRS ${CMAKE_CURRENT_BINARY_DIR}/caffe.pb.h)
  add_definitions(-DHAVE_PROTOBUF=1)

  add_custom_command(
    OUTPUT ${PROTOBUF_SRCS} ${PROTOBUF_HDRS}
    COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_CURRENT_SOURCE_DIR}/src/caffe/compiled/caffe.tar.gz
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Unpacking compiled caffe protobuf files"
    VERBATIM
  )
  set_source_files_properties(${PROTOBUF_SRCS} ${PROTOBUF_HDRS} PROPERTIES GENERATED TRUE)

  ocv_warnings_disable(CMAKE_CXX_FLAGS -Wunused-parameter)
  ocv_warnings_disable(CMAKE_CXX_FLAGS /wd4125 /wd4267 /wd4127 /wd4244 /wd4512 /wd4702)
endif()