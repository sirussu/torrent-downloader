cmake_minimum_required(VERSION 3.15)

project(torrent_pp)

# Set the output directory based on the platform
if(WIN32)
    set(OUTPUT_DIR "win32-x64")
elseif(APPLE)
    set(OUTPUT_DIR "macos-x64")
else()
    set(OUTPUT_DIR "linux-x64")
endif()

# Platform-specific defines
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601 -DWIN32)
endif()

# Boost
set(BOOST_DEBUG ON)  # Consider making this optional with a CMake option

find_package(Boost 1.81.0 REQUIRED system) # Simplified Boost finding
include_directories(${Boost_INCLUDE_DIRS})

# OpenSSL
find_package(OpenSSL REQUIRED)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(BUILD_SHARED_LIBS OFF)  # Consistent with original code

# Subdirectories
add_subdirectory(libtorrent)
add_subdirectory(fmt)

# Source files
file(GLOB SOURCE_FILES "src/*")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# C++ Standard
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/libtorrent/include")

# Link libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME} crypt32 OpenSSL::SSL OpenSSL::Crypto Boost::system torrent-rasterbar fmt)
else()
    target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto Boost::system torrent-rasterbar fmt)
    # Consider adding threading library explicitly for non-Windows, e.g., -pthread
    # target_link_libraries(${PROJECT_NAME} pthread) # If needed
endif()


# Remove platform-specific compiler flag modifications.  CMake handles these automatically.
# The original code tried to change /MD to /MT, which is generally not recommended
# and can lead to issues.  Let CMake decide the appropriate runtime library.