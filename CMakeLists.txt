cmake_minimum_required(VERSION 3.5)
project(rawx-cpp)

include(ExternalProject)
find_package(PkgConfig)

set(LINKER_LANGUAGE CXX)

macro(dump_dependency_components _DEP)
  message("${_DEP}")
  message(" system:${${_DEP}_SYSTEM} guess:${${_DEP}_GUESS} exe:${$_DEP}_EXE}")
  message(" incdir: ${${_DEP}_INCLUDE_DIRS}")
  message(" libdir: ${${_DEP}_LIBRARY_DIRS}")
  message(" lib: ${${_DEP}_LIBRARIES}")
endmacro()

if (NOT DEFINED CPPLINT_EXE)
  find_program(CPPLINT_EXE cpplint)
endif()
if (NOT DEFINED CPPCHECK_EXE)
  find_program(CPPCHECK_EXE cppcheck)
endif()

option(SYS "General trigger superseding all *_SYSTEM options" ON)
option(GUESS "General trigger superseding all *_GUESS options" ON)
option(TESTING "Enables the tests and their dependencies" ON)

################################################################################
### Google Log

option(GLOG_SYSTEM "Use system's googlelog" ON)
option(GLOG_GUESS "Try to find googlelog in standard places" OFF)
if (DEFINED GLOG_INCDIR AND DEFINED GLOG_LIBDIR)
    find_library(GLOG_LIBRARIES
            NAMES glog
            PATHS ${GLOG_LIBDIR})
	find_path(GLOG_INCLUDE_DIRS
            NAMES glog/logging.h
            PATHS ${GLOG_INCDIR})
elseif (SYS AND GLOG_SYSTEM)
    pkg_check_modules(GLOG libglog REQUIRED)
elseif (GUESS AND GLOG_GUESS)
    find_library(GLOG_LIBRARIES
            NAMES glog
            HINTS /usr/lib /usr/lib64)
	find_path(GLOG_INCLUDE_DIRS
            NAMES glog/logging.h
            PATHS /usr/include)
else ()
    ExternalProject_Add(googlelog
            DOWNLOAD_COMMAND ""
            SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rd/glog
            BINARY_DIR ${CMAKE_BINARY_DIR}/3rd/glog-bin
            TMP_DIR    ${CMAKE_BINARY_DIR}/3rd/glog-tmp
            STAMP_DIR  ${CMAKE_BINARY_DIR}/3rd/glog-stamp
            CMAKE_ARGS
            -DBUILD_SHARED_LIBS=ON
            -DWITH_THREADS=ON
            -DWITH_GFLAGS=OFF
            -DCMAKE_CXX_FLAGS=${MSVC_COMPILER_DEFS}
            PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
            INSTALL_COMMAND "")
    ExternalProject_Get_Property(googlelog source_dir)
    ExternalProject_Get_Property(googlelog binary_dir)
    set(GLOG_INCLUDE_DIRS ${source_dir}/src ${binary_dir})
    set(GLOG_LIBRARY_DIRS ${binary_dir})
    set(GLOG_LIBRARIES glog)
endif ()
dump_dependency_components("GLOG")

################################################################################
### Google Flags
### Often poorly packaged. The .pc file lacks, e.g. on Ubuntu 15. We accept
### system configuration, explicit paths, or paths guessed at standard places.

option(GFLAGS_SYSTEM "Use system's gflags" ON)
option(GFLAGS_GUESS "Try to find gflags in standard places" OFF)
if (DEFINED GFLAGS_INCDIR AND DEFINED GFLAGS_LIBDIR)
    find_library(GFLAGS_LIBRARIES
            NAMES gflags
            PATHS ${GFLAGS_LIBDIR})
    find_path(GFLAGS_INCLUDE_DIRS
            NAMES gflags/gflags.h
            PATHS ${GFLAGS_INCDIR})
elseif (SYS AND GFLAGS_SYSTEM)
    pkg_check_modules(GFLAGS gflags REQUIRED)
elseif (GUESS AND GFLAGS_GUESS)
    find_library(GFLAGS_LIBRARIES
            NAMES gflags
            HINTS /usr/lib /usr/lib64)
    find_path(GFLAGS_INCLUDE_DIRS
            NAMES gflags/gflags.h
            PATHS /usr/include)
else ()
    ExternalProject_Add(googleflags
            DOWNLOAD_COMMAND ""
            SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rd/gflags
            BINARY_DIR ${CMAKE_BINARY_DIR}/3rd/gflags-bin
            TMP_DIR    ${CMAKE_BINARY_DIR}/3rd/gflags-tmp
            STAMP_DIR  ${CMAKE_BINARY_DIR}/3rd/gflags-stamp
            CMAKE_ARGS
            -DBUILD_SHARED_LIBS=ON
            -DCMAKE_CXX_FLAGS=${MSVC_COMPILER_DEFS}
            PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
            INSTALL_COMMAND "")
    ExternalProject_Get_Property(googleflags source_dir)
    ExternalProject_Get_Property(googleflags binary_dir)
    set(GFLAGS_INCLUDE_DIRS ${binary_dir}/include)
    set(GFLAGS_LIBRARY_DIRS ${binary_dir}/lib)
    set(GFLAGS_LIBRARIES gflags)
endif ()
dump_dependency_components("GFLAGS")

################################################################################
### Google Test
### Worst of all, there is probably a good reason to explain gtest's packages
### only ship headers and sources...

option(GTEST_SYSTEM "Use system's googletest" ON)
option(GTEST_GUESS "Try to find googletest in standard places" OFF)
if (DEFINED GTEST_INCDIR AND DEFINED GTEST_LIBDIR)
    find_library(GTEST_LIBRARIES
            NAMES gtest
            PATHS ${GTEST_LIBDIR})
    find_path(GTEST_INCLUDE_DIRS
            NAMES gtest/gtest.h
            PATHS ${GTEST_INCDIR})
elseif (SYS AND GTEST_SYSTEM)
    pkg_check_modules(GTEST libgtest REQUIRED)
elseif (GUESS AND GTEST_GUESS)
    find_library(GTEST_LIBRARIES
            NAMES gtest
            HINTS /usr/lib /usr/lib64)
    find_path(GTEST_INCLUDE_DIRS
            NAMES gtest/gtest.h
            PATHS /usr/include)
else ()
    ExternalProject_Add(googletest
            DOWNLOAD_COMMAND ""
            SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rd/gtest
            BINARY_DIR ${CMAKE_BINARY_DIR}/3rd/gtest-bin
            TMP_DIR    ${CMAKE_BINARY_DIR}/3rd/gtest-tmp
            STAMP_DIR  ${CMAKE_BINARY_DIR}/3rd/gtest-stamp
            CMAKE_ARGS
            -DBUILD_SHARED_LIBS=ON
            -DBUILD_GTEST=ON
            -DBUILD_GMOCK=ON
            -DCMAKE_CXX_FLAGS=${MSVC_COMPILER_DEFS}
            PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
            INSTALL_COMMAND "")
    ExternalProject_Get_Property(googletest source_dir)
    ExternalProject_Get_Property(googletest binary_dir)
    set(GTEST_INCLUDE_DIRS ${source_dir}/googletest/include)
    set(GTEST_LIBRARY_DIRS ${binary_dir}/googletest)
    set(GTEST_LIBRARIES gtest)
endif ()
dump_dependency_components("GTEST")

################################################################################
### ATTR
option(ATTR_SYSTEM "Use system's libattr" ON)
option(ATTR_GUESS "Guess the libattr places at system's standards" ON)
if (DEFINED ATTR_INCDIR AND DEFINED ATTR_LIBDIR)
    find_library(ATTR_LIBRARIES
            NAMES attr
            PATHS ${ATTR_LIBDIR})
    find_path(ATTR_INCLUDE_DIRS
            NAMES attr/xattr.h
            PATHS ${ATTR_INCDIR})
elseif (SYS AND ATTR_SYSTEM)
    pkg_check_modules(ATTR libattr REQUIRED)
elseif (GUESS AND ATTR_GUESS)
    find_library(ATTR_LIBRARIES
            NAMES attr
            HINTS /usr/lib /usr/lib64)
    find_path(ATTR_INCLUDE_DIRS
            NAMES attr/xattr.h
            PATHS /usr/include)
else ()
    set(ATTR_INCLUDE_DIRS ATTR_INCLUDE_DIRS-NOTFOUND)
    set(ATTR_LIBRARIES ATTR_LIBRARIES-NOTFOUND)
endif ()
dump_dependency_components("ATTR")

################################################################################
### Proxygen
### It's rare that it is already on the system so by default we will construct
### it.
option(PROXYGEN_SYSTEM "Use system's Proxygen" OFF)
option(PROXYGEN_GUESS "Guess the Proxygen places at system's standard" OFF)
if (DEFINED PROXYGEN_INCDIR AND DEFINED PROXYGEN_LIBDIR)
  find_library(PROXYGENCURL_LIBRARIES
    NAMES proxygencurl
    PATHS ${PROXYGEN_LIBDIR})
  find_library(PROXYGENHTTPSERVER_LIBRARIES
    NAMES proxygenhttpserver
    PATHS ${PROXYGEN_LIBDIR})
  find_library(PROXYGEN_INCLUDE_DIRS
    NAMES proxygen
    PATHS ${GLOG_INCDIR})
elseif (SYS AND PROXYGEN_SYSTEM)
  pkg_check_modules(PROXYGENCURL libproxygencurl REQUIRED)
  pkg_check_modules(PROXYGENHTTPSERVER libproxygenhttpserver REQUIRED)
elseif (GUESS AND PROXYGEN_GUESS)
  find_library(PROXYGENCURL_LIBRARIES
    NAMES proxygencurl
    HINTS /usr/lib /usr/lib64 /usr/local/lib)
  find_library(PROXYGENHTTPSERVER_LIBRARIES
    NAMES proxygenhttpserver
    HINTS /usr/lib /usr/lib64 /usr/local/lib)
  find_path(PROXYGEN_INCLUDE_DIRS
    NAME proxygen
    HINTS /usr/include /usr/local/include)
else()
  execute_process(COMMAND "test -e build_already_done || (./deps.sh && touch build_already_done)"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/3rd/proxygen/proxygen)
  find_library(PROXYGENCURL_LIBRARIES
    NAMES proxygencurl
    HINTS /usr/local/lib)
  find_library(PROXYGENHTTPSERVER_LIBRARIES
    NAMES proxygenhttpserver
    HINTS  /usr/local/lib)
  find_path(PROXYGEN_INCLUDE_DIRS
    NAME proxygen
    HINTS  /usr/local/include )
endif()
dump_dependency_components("PROXYGEN")
dump_dependency_components("PROXYGENCURL")
dump_dependency_components("PROXYGENHTTPSERVER")

################################################################################
### Facebook Folly
### It is build with proxygen
option(FOLLY_SYSTEM "Use system's Folly" OFF)
option(FOLLY_GUESS "Guess the Folly places at system's standard" OFF)
if (DEFINED FOLLY_INCDIR AND DEFINED FOLLY_LIBDIR)
  find_library(FOLLY_LIBRARIES
    NAMES folly
    PATHS ${FOLLY_LIBDIR})
  find_path(FOLLY_INCLUDE_DIRS
    NAME folly
    PATHS ${FOLLY_INCDIR})
elseif(SYS AND FOLLY_SYSTEM)
  pkg_check_modules(FOLLY libfolly REQUIRED)
elseif(GUESS AND FOLLY_GUESS)
  find_library(FOLLY_LIBRARIES
    NAMES folly
    HINTS /usr/lib /usr/lib64 /usr/local/lib)
  find_path(FOLLY_INCLUDE_DIRS
    NAMES folly
    HINTS /usr/include /usr/local/include)
else()
  find_library(FOLLY_LIBRARIES
    NAMES folly
    HINTS /usr/local/lib)
  find_path(FOLLY_INCLUDE_DIRS
    NAMES folly
    HINTS /usr/local/include)
endif()
dump_dependency_components("FOLLY")

################################################################################
### Wangle
### It is build with proxygen
option(WANGLE_SYSTEM "Use system's Wangle" OFF)
option(WANGLE_GUESS "Guess the Wangle places at system's standard" OFF)
if (DEFINED WANGLE_INCDIR AND DEFINED WANGLE_LIBDIR)
  find_library(WANGLE_LIBRARIES
    NAMES wangle
    PATHS ${WANGLE_LIBDIR})
  find_path(WANGLE_INCLUDE_DIRS
    NAMES wangle
    PATHS ${WANGLE_INCDIR})
elseif( SYS AND WANGLE_SYSTEM)
  pkg_check_modules(WANGLE libwangle REQUIRED)
elseif( GUESS AND WANGLE_GUESS)
  find_library(WANGLE_LIBRARIES
    NAMES wangle
    HINTS /usr/lib /usr/lib64 /usr/local/lib)
  find_path(WANGLE_INCLUDE_DIRS
    NAMES wangle
    HINTS /usr/include /usr/local/include)
else()
  find_library(WANGLE_LIBRARIES
    NAMES wangle
    HINTS /usr/local/lib)
  find_path(WANGLE_INCLUDE_DIRS
    NAMES wangle
    HINTS /usr/local/include)
endif()
dump_dependency_components("WANGLE")
################################################################################

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -pipe -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wclobbered")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++14")

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -fno-inline")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2")

add_subdirectory(src)

if(TESTING)
  ENABLE_TESTING()
  add_subdirectory(tests/unit)
  add_subdirectory(tests/func)
endif()
    
