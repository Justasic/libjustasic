cmake_minimum_required(VERSION 2.8)

# Force external build
if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR} AND NOT WIN32)
    message(FATAL_ERROR "You can not use CMake to build from the root of it's source tree! Remove the CMakeCache.txt file from this directory, then create a separate directory (either below this directory or elsewhere), and then re-run CMake from there.")
endif(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR} AND NOT WIN32)

# Project version
SET(PROJECT_MAJOR_VERSION 1)
SET(PROJECT_MINOR_VERSION 0)
SET(PROJECT_PATCH_LEVEL 0)

# Get the git revision location for the branch we're on
if(EXISTS "${CMAKE_SOURCE_DIR}/.git/HEAD")
        file(READ ${CMAKE_SOURCE_DIR}/.git/HEAD GIT_HEAD_LOC)
        string(LENGTH ${GIT_HEAD_LOC} HEAD_LEN)
        math(EXPR LEN "${HEAD_LEN} - 5")
        string(SUBSTRING ${GIT_HEAD_LOC} 5 ${LEN} GIT_HEAD)
        # Weird nastery to remove newlines which screw up the if statement below.
        set(GIT_SHA_PATH "${CMAKE_SOURCE_DIR}/.git/${GIT_HEAD}")
        string(REGEX REPLACE "(\r?\n)+$" "" GIT_SHA_PATH "${GIT_SHA_PATH}")
endif(EXISTS "${CMAKE_SOURCE_DIR}/.git/HEAD")

# Get the git revision we're on for the version string
if(EXISTS "${GIT_SHA_PATH}")
        file(READ "${GIT_SHA_PATH}" VERSION_STR)
        string(REGEX REPLACE "(\r?\n)+$" "" VERSION_STR "${VERSION_STR}")
        # Get the length of the string
        string(LENGTH ${VERSION_STR} VERSION_LEN)
        # Subtract 7 from the string's length
        math(EXPR VERSION_NUM_LEN "${VERSION_LEN} - ${VERSION_LEN} + 7")
        # Extract the value from the string
        string(SUBSTRING ${VERSION_STR} 0 ${VERSION_NUM_LEN} VERSION_GIT)
endif(EXISTS "${GIT_SHA_PATH}")

# Set our version strings
SET(VERSION_SIMPLE "${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_PATCH_LEVEL}")
SET(VERSION_FULL "${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_PATCH_LEVEL}-${VERSION_GIT}")

# Cmake module path (if exists)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

file(GLOB SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")
list(SORT SOURCE_FILES)

# Find clang (if the system has it)
find_program(CLANG clang)
find_program(CLANGXX clang++)

# prefer clang instead of gcc (or whatever shit compiler they're using) This can be disabled with
# the NO_CLANG option
option(NO_CLANG "Don't prefer clang for compilation" OFF)
if (NOT NO_CLANG)
	if (CLANG)
		set(CMAKE_C_COMPILER ${CLANG})
	endif (CLANG)

	# In case we need to compile C++ in the future (which I doubt we will)
	if (CLANGXX)
		set(CMAKE_CXX_COMPILER ${CLANGXX})
	endif (CLANGXX)
endif (NOT NO_CLANG)

# Finally initialize our project
project(justasic CXX)
enable_language(C)

message(STATUS "${PROJECT_NAME} version: ${VERSION_FULL}")

# Add our include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Check for platform-specific things we need
include (CheckTypeSize)
include (CheckIncludeFile)
include (CheckLibraryExists)
include (CheckFunctionExists)
include (CheckCXXSourceCompiles)
include (CheckCXXCompilerFlag)

check_type_size(int8_t HAVE_INT8_T)
check_type_size(uint8_t HAVE_UINT8_T)
check_type_size(int16_t HAVE_INT16_T)
check_type_size(uint16_t HAVE_UINT16_T)
check_type_size(uint32_t HAVE_UINT32_T)
check_type_size(int64_t HAVE_INT64_T)
check_type_size(uint64_t HAVE_UINT64_T)

check_type_size(u_int8_t HAVE_U_INT8_T)
check_type_size(u_int16_t HAVE_U_INT16_T)
check_type_size(u_int32_t HAVE_U_INT32_T)
check_type_size(u_int64_t HAVE_U_INT64_T)

check_include_file(sys/epoll.h HAVE_SYS_EPOLL_H)
check_include_file(setjmp.h HAVE_SETJMP_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(stddef.h HAVE_STDDEF_H)

check_cxx_compiler_flag(-std=c++11 HAVE_CXX11_FLAG)

# make sure we support C++11 or fail.
if(NOT HAVE_CXX11_FLAG)
	message(FATAL_ERROR "Your compiler (${CMAKE_CXX_COMPILER}) must support C++11 (or the -std=c++11 flag)")
else(NOT HAVE_CXX11_FLAG)
	set(CFLAGS "${CFLAGS} -std=c++11")
endif(NOT HAVE_CXX11_FLAG)

# Because strndupa is apparnetly not a function or some shit, we must
# make sure this program compiles.
check_cxx_source_compiles("
#define _GNU_SOURCE 1
#include <string.h>
int main() { const char *abc = \"abc\"; char *str = strndupa(abc, 5); return 0; }
" HAVE_STRNDUPA)

# Probably a better way of doing this but whatever.
set(LIBNAME "lib${PROJECT_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")

# Finally, tell CMake how to build the project
add_library(${LIBNAME} STATIC ${SOURCE_FILES})
set_source_files_properties(${SOURCE_FILES} PROPERTIES LANGUAGE CXX COMPILE_FLAGS "${CFLAGS}")
set_target_properties(${LIBNAME} PROPERTIES LINKER_LANGUAGE CXX PREFIX "" SUFFIX "" LINK_FLAGS "${LINKFLAGS}")

# Do the make install
install(TARGETS ${LIBNAME} ARCHIVE DESTINATION lib)