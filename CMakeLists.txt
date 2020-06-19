cmake_minimum_required(VERSION 2.8)

# Force external build
if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR} AND NOT WIN32)
    message(FATAL_ERROR "You can not use CMake to build from the root of it's source tree! Remove the CMakeCache.txt file from this directory, then create a separate directory (either below this directory or elsewhere), and then re-run CMake from there.")
endif(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR} AND NOT WIN32)

# Project version
SET(PROJECT_MAJOR_VERSION 1)
SET(PROJECT_MINOR_VERSION 0)
SET(PROJECT_PATCH_LEVEL 0)

# Cmake module path (if exists)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
# include our function checks
include(systemfuncs)

# Get the git revision location for the branch we're on
GetGitRevision()

# Set our version strings
SET(VERSION_SIMPLE "${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_PATCH_LEVEL}")
SET(VERSION_FULL "${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_PATCH_LEVEL}-${GIT_REVISION_SHORT}")

message(STATUS "VERSION: ${VERSION_FULL}")

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

# Make sure the compiler accepts C11
list(APPEND CFLAGS -Werror=implicit-function-declaration -Wall -Wextra -Wno-unused-parameter -fsanitize-address-use-after-scope)
#list(APPEND CFLAGS -Wdocumentation)
if (NOT NO_CLANG)
	# Because of C++17 being somewhat incomplete for clang/g++ we
	# have to make clang use libc++ instead of glibc++
	find_library(LIBCPP libc++ REQUIRED)
    LIST(APPEND CFLAGS -stdlib=libc++ -fsanitize=address -fno-omit-frame-pointer)
    LIST(APPEND LINKFLAGS -fsanitize=address -stdlib=libc++ -fuse-ld=lld)
endif (NOT NO_CLANG)

if(${CMAKE_BUILD_TYPE} MATCHES "Release")
	add_definitions(-D_FORTIFY_SOURCE=2)
	list(APPEND LINKFLAGS -s)
endif(${CMAKE_BUILD_TYPE} MATCHES "Release")

# Check for platform-specific things we need
include (CheckTypeSize)
include (CheckIncludeFile)
include (CheckLibraryExists)
include (CheckFunctionExists)
include (CheckCXXSourceCompiles)
include (CheckCXXCompilerFlag)
include (ExternalProject)
include (TestBigEndian)

check_type_size(int8_t HAVE_INT8_T)
check_type_size(uint8_t HAVE_UINT8_T)
check_type_size(int16_t HAVE_INT16_T)
check_type_size(uint16_t HAVE_UINT16_T)
check_type_size(uint32_t HAVE_UINT32_T)
check_type_size(int64_t HAVE_INT64_T)
check_type_size(uint64_t HAVE_UINT64_T)
check_type_size(intptr_t HAVE_INTPTR_T)

check_type_size(u_int8_t HAVE_U_INT8_T)
check_type_size(u_int16_t HAVE_U_INT16_T)
check_type_size(u_int32_t HAVE_U_INT32_T)
check_type_size(u_int64_t HAVE_U_INT64_T)

check_include_file(sys/epoll.h HAVE_SYS_EPOLL_H)
check_include_file(setjmp.h HAVE_SETJMP_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(stddef.h HAVE_STDDEF_H)
check_include_file(dlfcn.h HAVE_DLFCN_H)

# Because strndupa is apparnetly not a function or some shit, we must
# make sure this program compiles.
check_cxx_source_compiles("
#define _GNU_SOURCE 1
#include <string.h>
int main() { const char *abc = \"abc\"; char *str = strndupa(abc, 5); return 0; }
" HAVE_STRNDUPA)

###########################################################
# Library section
###########################################################

# ================= Nlohmann's JSON lob ==============
set(JSON_ROOT ${CMAKE_BINARY_DIR}/external/json)
set(JSON_CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_INSTALL_PREFIX=${JSON_ROOT} -DCMAKE_CXX_FLAGS=${SUBPROJ_CXX_FLAGS})
ExternalProject_Add(nlohmannjson
	PREFIX ${JSON_ROOT}
	GIT_REPOSITORY https://github.com/nlohmann/json.git
	GIT_SHALLOW 1
	TIMEOUT 10
	CMAKE_ARGS ${JSON_CMAKE_ARGS}
	LOG_DOWNLOAD ON

	CONFIGURE_COMMAND "" # Disable configure step
	BUILD_COMMAND "" # Disable build step
	INSTALL_COMMAND "" # Disable install step
	UPDATE_COMMAND "" # Disable update step, only clone once.
)
ExternalProject_Get_Property(nlohmannjson source_dir)
set (JSON_INCLUDE_DIR ${source_dir}/include/)

# ================= LibGuarded Threadling Lib ==============
set(LIBGUARDED_ROOT ${CMAKE_BINARY_DIR}/external/libguarded)
set(LIBGUARDED_CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_INSTALL_PREFIX=${LIBGUARDED_ROOT} -DCMAKE_CXX_FLAGS=${SUBPROJ_CXX_FLAGS})
ExternalProject_Add(libguarded
	PREFIX ${LIBGUARDED_ROOT}
	GIT_REPOSITORY https://github.com/copperspice/libguarded.git
	GIT_SHALLOW 1
	TIMEOUT 10
	CMAKE_ARGS ${LIBGUARDED_CMAKE_ARGS}
	LOG_DOWNLOAD ON

	CONFIGURE_COMMAND "" # Disable configure step
	BUILD_COMMAND "" # Disable build step
	INSTALL_COMMAND "" # Disable install step
	UPDATE_COMMAND "" # Disable update step, only clone once.
)
ExternalProject_Get_Property(libguarded source_dir)
set (LIBGUARDED_INCLUDE_DIR ${source_dir}/src)

# ================ backward library ===================
set(BACKWARD_ROOT ${CMAKE_BINARY_DIR}/external/backward)
set(BACKWARD_CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_INSTALL_PREFIX=${BACKWARD_ROOT} -DCMAKE_CXX_FLAGS=${SUBPROJ_CXX_FLAGS})
ExternalProject_Add(backward
	PREFIX ${BACKWARD_ROOT}
	GIT_REPOSITORY https://github.com/bombela/backward-cpp.git
	GIT_SHALLOW 1
	TIMEOUT 10
	CMAKE_ARGS ${BACKWARD_CMAKE_ARGS}
	LOG_DOWNLOAD ON
	LOG_BUILD ON
	CONFIGURE_COMMAND "" # Disable configure step
	BUILD_COMMAND "" # Disable build step
	INSTALL_COMMAND "" # Disable install step
	UPDATE_COMMAND "" # Disable update step, only clone once.
)
ExternalProject_Get_Property(backward source_dir)
set (BACKWARD_INCLUDE_DIR ${source_dir})

# ================ SPDLOG library ===================
set(SPDLOG_ROOT ${CMAKE_BINARY_DIR}/external/spdlog)
set(SPDLOG_CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_INSTALL_PREFIX=${SPDLOG_ROOT} -DCMAKE_CXX_FLAGS=${SUBPROJ_CXX_FLAGS})
ExternalProject_Add(spdlog
	PREFIX ${SPDLOG_ROOT}
	GIT_REPOSITORY https://github.com/gabime/spdlog.git
	GIT_SHALLOW 1
	TIMEOUT 10
	#BINARY_DIR ${SPDLOG_ROOT}
	#INSTALL_DIR ${SPDLOG_ROOT}
	CMAKE_ARGS ${SPDLOG_CMAKE_ARGS}
	LOG_DOWNLOAD ON
	LOG_BUILD ON
	CONFIGURE_COMMAND "" # Disable configure step
	BUILD_COMMAND "" # Disable build step
	INSTALL_COMMAND "" # Disable install step
	UPDATE_COMMAND "" # Disable update step, only clone once.
)
ExternalProject_Get_Property(spdlog source_dir)
set (SPDLOG_INCLUDE_DIR ${source_dir}/include)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Check endianness.
TEST_BIG_ENDIAN(BIGENDIAN)
if (BIGENDIAN)
	message(STATUS "Compiling for Big Endian...")
	add_definitions(-DBIGENDIAN=1)
else (BIGENDIAN)
	message(STATUS "Compiling for Little Endian...")
	add_definitions(-DBIGENDIAN=0)
endif(BIGENDIAN)

###########################################################
# Compilation section
###########################################################

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

# Check flex version to prevent yylex_destroy link errors.
if(${FLEX_VERSION} VERSION_LESS "2.5.9")
        message(FATAL_ERROR "Your flex version is ${FLEX_VERSION}, You must have flex version 2.5.9 or higher!")
endif(${FLEX_VERSION} VERSION_LESS "2.5.9")

# Generate our flex and bison targets.
FLEX_TARGET(LEXER ${CMAKE_SOURCE_DIR}/src/scanner.ll ${CMAKE_BINARY_DIR}/lexer.cpp)
BISON_TARGET(PARSER ${CMAKE_SOURCE_DIR}/src/parser.yy ${CMAKE_BINARY_DIR}/parser.cpp)
ADD_FLEX_BISON_DEPENDENCY(LEXER PARSER)

configure_file(${CMAKE_SOURCE_DIR}/cmake/sysconf.h.cmake ${CMAKE_BINARY_DIR}/sysconf.h)

include_directories(
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_BINARY_DIR}
	${SPDLOG_INCLUDE_DIR}
	${BACKWARD_INCLUDE_DIR}
	${JSON_INCLUDE_DIR}
	${LIBGUARDED_INCLUDE_DIR}
)

# Get our source C/C++ files
GetSources("${CMAKE_CURRENT_SOURCE_DIR}/src" SOURCE_FILES)
list(SORT SOURCE_FILES)

# Probably a better way of doing this but whatever.
set(LIBNAME "lib${PROJECT_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")

# Finally, tell CMake how to build the project
add_library(${LIBNAME} STATIC ${SOURCE_FILES} ${FLEX_LEXER_OUTPUTS} ${BISON_PARSER_OUTPUTS})

# Add our dependencies
add_dependencies(${LIBNAME} spdlog nlohmannjson libguarded backward)

# We use pretty much all of C++11, a lot of C++14, and quite a bit of C++17
# so try and require as much as we can so we can have C++17 support.
target_compile_features(${LIBNAME}
    PRIVATE
        cxx_thread_local
        cxx_user_literals
        cxx_variadic_macros
        cxx_variadic_templates
        cxx_rvalue_references
        cxx_right_angle_brackets
        cxx_range_for
        cxx_noexcept
        cxx_long_long_type
        cxx_lambdas
        cxx_deleted_functions
        cxx_delegating_constructors
        cxx_auto_type
        cxx_decltype
        cxx_constexpr
    PUBLIC
        cxx_nullptr
)

set_source_files_properties(${SOURCE_FILES} PROPERTIES LANGUAGE CXX)
set_target_properties(${LIBNAME}
	PROPERTIES
		LINKER_LANGUAGE CXX
		PREFIX ""
		SUFFIX ""
		LINK_FLAGS "${LINKFLAGS}"
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO
)
target_compile_options(${LIBNAME} PRIVATE ${CFLAGS})

###########################################################
# Installation section
###########################################################

# Do the make install
install(TARGETS ${LIBNAME} ARCHIVE DESTINATION lib)
# Grab all source files
file(GLOB HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "include/*.h")
list(SORT HEADER_FILES)
install(FILES ${HEADER_FILES} DESTINATION include/justasic)

include(InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_MAJOR_VERSION}")
set (CPACK_PACKAGE_VERSION_MINOR "${PROJECT_MINOR_VERSION}")
set (CPACK_PACKAGE_VERSION_PATCH "${PROJECT_PATCH_LEVEL}")
set (CPACK_PACKAGE_NAME "libjustasic")
set (CPACK_PACKAGE_VENDOR "Justin Crawford")
include(CPack)
