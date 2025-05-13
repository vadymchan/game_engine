cmake_minimum_required(VERSION 3.26)
project(asset_tools NONE)

set(TOOLS_BIN_DIR "${CMAKE_BINARY_DIR}/asset_tools")
file(MAKE_DIRECTORY "${TOOLS_BIN_DIR}")

include(ExternalProject)

ExternalProject_Add(toktx
  GIT_REPOSITORY    https://github.com/KhronosGroup/KTX-Software.git
  GIT_TAG           v4.4.0
  GIT_SHALLOW       TRUE
  GIT_PROGRESS      TRUE
  UPDATE_COMMAND    ""
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=Release
    -DKTX_FEATURE_TOOLS=ON
    -DKTX_FEATURE_LIBRARIES=OFF
    -DKTX_FEATURE_TESTS=OFF
    -DBUILD_SHARED_LIBS=OFF
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${TOOLS_BIN_DIR}
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG=${TOOLS_BIN_DIR}
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=${TOOLS_BIN_DIR}
  INSTALL_COMMAND   ""
)

ExternalProject_Get_Property(toktx SOURCE_DIR)
set(BASISU_PATH "${SOURCE_DIR}/external/basisu")

ExternalProject_Add(gltfpack
  GIT_REPOSITORY    https://github.com/zeux/meshoptimizer.git
  GIT_TAG           v0.23
  GIT_SHALLOW       TRUE
  GIT_PROGRESS      TRUE
  UPDATE_COMMAND    "" 
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=Release
    -DMESHOPT_BUILD_GLTFPACK=ON
    -DMESHOPT_INSTALL=OFF
    -DMESHOPT_BASISU_PATH=${BASISU_PATH}
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${TOOLS_BIN_DIR}
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG=${TOOLS_BIN_DIR}
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=${TOOLS_BIN_DIR}
  INSTALL_COMMAND   ""
)


add_custom_target(asset_tools
  DEPENDS gltfpack toktx
  COMMENT "Build offline asset-pipeline tools"
)
