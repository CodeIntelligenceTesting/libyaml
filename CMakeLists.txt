
cmake_minimum_required(VERSION 3.16)
project (ci LANGUAGES C CXX)

find_package(cifuzz NO_SYSTEM_ENVIRONMENT_PATH)
enable_fuzz_testing()

set (CI_VERSION_MAJOR 0)
set (CI_VERSION_MINOR 2)
set (CI_VERSION_PATCH 5)
set (CI_VERSION_STRING "${CI_VERSION_MAJOR}.${CI_VERSION_MINOR}.${CI_VERSION_PATCH}")

option(BUILD_SHARED_LIBS "Build libci as a shared library" OFF)
set(CI_STATIC_LIB_NAME "ci" CACHE STRING "Base name of static library output")

#
# Output directories for a build tree
#
if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

#
# Install relative directories
#
if(NOT DEFINED INSTALL_LIB_DIR)
  set(INSTALL_LIB_DIR lib)
endif()
if(NOT DEFINED INSTALL_BIN_DIR)
  set(INSTALL_BIN_DIR bin)
endif()
if(NOT DEFINED INSTALL_INCLUDE_DIR)
  set(INSTALL_INCLUDE_DIR include)
endif()
if(NOT DEFINED INSTALL_CMAKE_DIR)
  set(INSTALL_CMAKE_DIR cmake)
endif()

#
# Build library
#
set(SRCS
  src/api.c
  src/dumper.c
  src/emitter.c
  src/loader.c
  src/parser.c
  src/reader.c
  src/scanner.c
  src/writer.c
  )

set(config_h ${CMAKE_CURRENT_BINARY_DIR}/include/config.h)
configure_file(
  cmake/config.h.in
  ${config_h}
  )

add_library(ci ${SRCS})

if(NOT BUILD_SHARED_LIBS)
  set_target_properties(ci
    PROPERTIES OUTPUT_NAME ${CI_STATIC_LIB_NAME}
  )
endif()

set_target_properties(ci
  PROPERTIES DEFINE_SYMBOL CI_DECLARE_EXPORT
  )

target_compile_definitions(ci
  PRIVATE HAVE_CONFIG_H
  PUBLIC
    $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:CI_DECLARE_STATIC>
    $<$<BOOL:${MSVC}>:_CRT_SECURE_NO_WARNINGS>
  )

target_include_directories(ci PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
  )

#
# Install rules
#
install(
  FILES
    include/ci.h
  DESTINATION include COMPONENT Development
  )

install(
  TARGETS ci
  EXPORT ciTargets
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT Runtime
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT Development
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT Development
  )

#
# Add tests
#
include(CTest) # This module defines BUILD_TESTING option
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

#
# Generate 'ciConfig.cmake', 'ciConfigVersion.cmake' and 'ciTargets.cmake'
#
include(CMakePackageConfigHelpers)

# Configure 'ciConfig.cmake' for a build tree
set(CONFIG_DIR_CONFIG ${PROJECT_BINARY_DIR})
set(config_file ${PROJECT_BINARY_DIR}/ciConfig.cmake)
configure_package_config_file(
  ciConfig.cmake.in
  ${config_file}
  INSTALL_DESTINATION ${PROJECT_BINARY_DIR}
  PATH_VARS CONFIG_DIR_CONFIG
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
  )

# Configure 'ciTargets.cmake' for a build tree
export(TARGETS ci
  FILE ${PROJECT_BINARY_DIR}/ciTargets.cmake
  )

# Configure and install 'ciConfig.cmake' for an install tree
set(CONFIG_DIR_CONFIG ${INSTALL_CMAKE_DIR})
set(install_config_file ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/ciConfig.cmake )
configure_package_config_file(
  ciConfig.cmake.in
  ${install_config_file}
  INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/${INSTALL_CMAKE_DIR}
  PATH_VARS CONFIG_DIR_CONFIG
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
  )
install(
  FILES ${install_config_file}
  DESTINATION ${INSTALL_CMAKE_DIR} COMPONENT Development
  )

# Configure and install 'ciTargets.cmake' for an install tree
install(EXPORT ciTargets
  FILE ciTargets.cmake
  DESTINATION ${INSTALL_CMAKE_DIR}
  COMPONENT Development
  )

# Configure 'ciConfigVersion.cmake' for a build tree
set(config_version_file ${PROJECT_BINARY_DIR}/ciConfigVersion.cmake)
write_basic_package_version_file(
    ${config_version_file}
    VERSION ${CI_VERSION_STRING}
    COMPATIBILITY AnyNewerVersion
)
# ... and install for an install tree
install(
  FILES ${config_version_file}
  DESTINATION ${INSTALL_CMAKE_DIR} COMPONENT Development
  )

add_fuzz_test(fuzz fuzz.cpp)
target_link_libraries(fuzz PRIVATE ci)

add_subdirectory(cifuzz-spark)
