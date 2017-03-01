include_directories(BEFORE
  ${CMAKE_SOURCE_DIR}/
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/3rd
  ${CMAKE_BINARY_DIR}
  ${GTEST_INCLUDE_DIRS}
  ${GFLAGS_INCLUDE_DIRS}
  ${GLOG_INCLUDE_DIRS}
  ${WANGLE_INCLUDE_DIRS}
  ${FOLLY_INCLUDE_DIRS}
  ${PROXYGEN_INCLUDE_DIRS})

link_directories(
  ${GTEST_LIBRARY_DIRS}
  ${GFLAGS_LIBRARY_DIRS}
  ${GLOG_LIBRARY_DIRS}
  ${FOLLY_LIBRARY_DIRS}
  ${WANGLE_LIBRARY_DIRS}
  ${PROXYGENCURL_LIBRARY_DIRS}
  ${PROXYGENHTTPSERVER_LIBRARY_DIRS})


add_executable(test-xattr TestXAttr.cpp)
target_link_libraries(test-xattr rawx-utils ${GTEST_LIBRARIES} ${GFLAGS_LIBRARIES} ${GLOG_LIBRARIES})
add_test(NAME unit/xattr COMMAND test-xattr)

add_executable(test-blob TestBlob.cpp)
target_link_libraries(test-blob rawx-blob rawx-utils ${GLOG_LIBRARIES} ${GFLAGS_LIBRARIES} ${GTEST_LIBRARIES})
add_test(NAME unit/blob COMMAND test-blob)

add_executable(test-rawx TestRawx.cpp)
target_link_libraries(test-rawx rawx-server rawx-blob rawx-utils ${GTEST_LIBRARIES} ${GFLAGS_LIBRARIES} ${GLOG_LIBRARIES}
  ${PROXYGENHTTPSERVER_LIBRARIES} ${PROXYGENCURL_LIBRARIES} ${WANGLE_LIBRARIES} ${FOLLY_LIBRARIES}) 
add_test(NAME unit/rawx COMMAND test-rawx)

if (CPPLINT_EXE)
	file(GLOB_RECURSE files RELATIVE "${CMAKE_SOURCE_DIR}"
		${CMAKE_SOURCE_DIR}/src/*.h
		${CMAKE_SOURCE_DIR}/src/*.c
		${CMAKE_SOURCE_DIR}/tests/common/*.h
		${CMAKE_SOURCE_DIR}/tests/unit/*.h
		${CMAKE_SOURCE_DIR}/tests/func/*.c
		${CMAKE_SOURCE_DIR}/bin/*.h
		${CMAKE_SOURCE_DIR}/bin/*.c
		${CMAKE_SOURCE_DIR}/src/*.hpp
		${CMAKE_SOURCE_DIR}/src/*.cpp
		${CMAKE_SOURCE_DIR}/tests/common/*.hpp
		${CMAKE_SOURCE_DIR}/tests/unit/*.cpp
		${CMAKE_SOURCE_DIR}/tests/func/*.cpp
		${CMAKE_SOURCE_DIR}/bin/*.hpp
		${CMAKE_SOURCE_DIR}/bin/*.cpp
		)
	      
	foreach(f ${files})
		add_test(NAME cpplint_format:${f}
			COMMAND ${CPPLINT_EXE} ${CMAKE_SOURCE_DIR}/${f})
	endforeach()
endif()

if (CPPCHECK_EXE) 
	file(GLOB_RECURSE files RELATIVE "${CMAKE_SOURCE_DIR}"
	  ${CMAKE_SOURCE_DIR}/src/*.h
		${CMAKE_SOURCE_DIR}/src/*.c
		${CMAKE_SOURCE_DIR}/tests/common/*.h
		${CMAKE_SOURCE_DIR}/tests/unit/*.h
		${CMAKE_SOURCE_DIR}/tests/func/*.c
		${CMAKE_SOURCE_DIR}/bin/*.h
		${CMAKE_SOURCE_DIR}/bin/*.c
		${CMAKE_SOURCE_DIR}/src/*.hpp
		${CMAKE_SOURCE_DIR}/src/*.cpp
		${CMAKE_SOURCE_DIR}/tests/common/*.hpp
		${CMAKE_SOURCE_DIR}/tests/unit/*.cpp
		${CMAKE_SOURCE_DIR}/tests/func/*.cpp
		${CMAKE_SOURCE_DIR}/bin/*.hpp
		${CMAKE_SOURCE_DIR}/bin/*.cpp)
	foreach(f ${files})
		add_test(NAME cppcheck_format:${f}
			COMMAND ${CPPCHECK_EXE} --enable=all ${CMAKE_SOURCE_DIR}/${f})
	endforeach()
endif()
