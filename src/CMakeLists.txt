include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${WANGLE_INCLUDE_DIRS}
  ${FOLLY_INCLUDE_DIRS}
  ${PROXYGEN_INCLUDE_DIRS})

link_directories(
  ${FOLLY_LIBRARY_DIRS}
  ${WANGLE_LIBRARY_DIRS}
  ${PROXYGENCURL_LIBRARY_DIRS}
  ${PROXYGENHTTPSERVER_LIBRARY_DIRS})

add_library(rawx-utils SHARED
  utils.hpp
  utils.cpp)
target_link_libraries(rawx-utils ${GTEST_LIBRARIES} ${GFLAGS_LIBRARIES} ${GLOG_LIBRARIES})
add_library(rawx-blob SHARED
  blob.hpp
  blob.cpp)
target_link_libraries(rawx-blob ${GTEST_LIBRARIES} ${GFLAGS_LIBRARIES} ${GLOG_LIBRARIES} crypto)
add_library(rawx-server SHARED
  rawx.hpp
  rawx.cpp)
target_link_libraries(rawx-server ${GTEST_LIBRARIES} ${GFLAGS_LIBRARIES} ${GLOG_LIBRARIES})

add_executable(main server.cpp rawx.cpp blob.cpp utils.cpp )
target_link_libraries(main ${GTEST_LIBRARIES} ${GFLAGS_LIBRARIES} ${GLOG_LIBRARIES} 
  ${PROXYGENHTTPSERVER_LIBRARIES} ${PROXYGENCURL_LIBRARIES} ${WANGLE_LIBRARIES} ${FOLLY_LIBRARIES}
  pthread crypto)
