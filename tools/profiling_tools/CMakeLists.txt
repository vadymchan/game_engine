cmake_minimum_required(VERSION 3.26)
project(profiling_tools NONE)

if(NOT DEFINED TRACY_VERSION)
    message(FATAL_ERROR "TRACY_VERSION must be defined by parent CMakeLists.txt")
endif()

message(STATUS "Using Tracy version: ${TRACY_VERSION}")

set(PROFILING_TOOLS_BIN_DIR "${CMAKE_BINARY_DIR}/profiling_tools")
file(MAKE_DIRECTORY "${PROFILING_TOOLS_BIN_DIR}")

if(WIN32)
    set(TRACY_PLATFORM "windows")
    set(TRACY_ARCHIVE_EXT ".zip")
    set(TRACY_EXECUTABLE_EXT ".exe")
else()
    message(FATAL_ERROR "Tracy profiling tools are currently supported only on Windows platform")
endif()

set(TRACY_BASE_URL "https://github.com/wolfpld/tracy/releases/download/${TRACY_VERSION}")

# remove 'v' prefix from version for archive name (v0.11.1 -> 0.11.1)
string(REGEX REPLACE "^v" "" TRACY_VERSION_NUMBER "${TRACY_VERSION}")

set(ARCHIVE_NAME "${TRACY_PLATFORM}-${TRACY_VERSION_NUMBER}${TRACY_ARCHIVE_EXT}")
set(DOWNLOAD_URL "${TRACY_BASE_URL}/${ARCHIVE_NAME}")
set(DOWNLOAD_PATH "${PROFILING_TOOLS_BIN_DIR}/${ARCHIVE_NAME}")

set(TRACY_TOOLS_TO_EXTRACT
    "tracy-profiler"     
    # "tracy-capture"        # automated/headless profiling
    # "tracy-csvexport"      # CSV export functionality
    # "tracy-import-chrome"  # Chrome import
    # "tracy-import-fuchsia" # Fuchsia import  
    # "tracy-update"         # Tracy update tool
)

set(NEED_DOWNLOAD FALSE)
foreach(TOOL ${TRACY_TOOLS_TO_EXTRACT})
    set(EXECUTABLE_PATH "${PROFILING_TOOLS_BIN_DIR}/${TOOL}${TRACY_EXECUTABLE_EXT}")
    if(NOT EXISTS "${EXECUTABLE_PATH}")
        set(NEED_DOWNLOAD TRUE)
        break()
    endif()
endforeach()

if(NEED_DOWNLOAD)
    message(STATUS "Downloading Tracy tools from ${DOWNLOAD_URL}")

    file(DOWNLOAD 
        "${DOWNLOAD_URL}" 
        "${DOWNLOAD_PATH}"
        SHOW_PROGRESS
        STATUS DOWNLOAD_STATUS
    )

    list(GET DOWNLOAD_STATUS 0 DOWNLOAD_ERROR_CODE)
    if(NOT DOWNLOAD_ERROR_CODE EQUAL 0)
        list(GET DOWNLOAD_STATUS 1 DOWNLOAD_ERROR_MESSAGE)
        message(FATAL_ERROR "Failed to download Tracy tools: ${DOWNLOAD_ERROR_MESSAGE}")
    endif()

    message(STATUS "Extracting Tracy tools")
    file(ARCHIVE_EXTRACT 
        INPUT "${DOWNLOAD_PATH}"
        DESTINATION "${PROFILING_TOOLS_BIN_DIR}/temp_extract"
    )

    foreach(TOOL ${TRACY_TOOLS_TO_EXTRACT})
        set(SOURCE_PATH "${PROFILING_TOOLS_BIN_DIR}/temp_extract/${TOOL}${TRACY_EXECUTABLE_EXT}")
        set(DEST_PATH "${PROFILING_TOOLS_BIN_DIR}/${TOOL}${TRACY_EXECUTABLE_EXT}")

        if(EXISTS "${SOURCE_PATH}")
            file(COPY "${SOURCE_PATH}" DESTINATION "${PROFILING_TOOLS_BIN_DIR}")
            message(STATUS "Tracy tool extracted: ${TOOL}${TRACY_EXECUTABLE_EXT}")
        else()
            message(WARNING "Could not find ${TOOL}${TRACY_EXECUTABLE_EXT} in downloaded archive")
        endif()
    endforeach()

    file(REMOVE_RECURSE "${PROFILING_TOOLS_BIN_DIR}/temp_extract")
    file(REMOVE "${DOWNLOAD_PATH}")
    
else()
    message(STATUS "All requested Tracy tools already exist in ${PROFILING_TOOLS_BIN_DIR}")
endif()

message(STATUS "Tracy profiling tools are ready in: ${PROFILING_TOOLS_BIN_DIR}")
message(STATUS "Available tools:")
foreach(TOOL ${TRACY_TOOLS_TO_EXTRACT})
    set(EXECUTABLE_PATH "${PROFILING_TOOLS_BIN_DIR}/${TOOL}${TRACY_EXECUTABLE_EXT}")
    if(EXISTS "${EXECUTABLE_PATH}")
        message(STATUS "  ${TOOL}${TRACY_EXECUTABLE_EXT}")
    endif()
endforeach()

# Optional: dummy target for manual rebuild if needed
add_custom_target(profiling_tools
    COMMAND ${CMAKE_COMMAND} -E echo "Tracy tools are already downloaded to ${PROFILING_TOOLS_BIN_DIR}"
    COMMENT "Tracy profiling tools location"
)