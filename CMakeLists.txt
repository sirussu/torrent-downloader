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

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

# Boost
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.74 REQUIRED COMPONENTS system)
target_link_libraries(torrent_pp PRIVATE Boost::system)

# OpenSSL
set(OPENSSL_USE_STATIC_LIBS ON)
find_package(OpenSSL REQUIRED)

# Threads
find_package(Threads REQUIRED)
target_link_libraries(torrent_pp PRIVATE Threads::Threads)

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
set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
        "${CMAKE_SOURCE_DIR}/libtorrent/include"
)

# Link libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME}
            crypt32
            OpenSSL::SSL
            OpenSSL::Crypto
            Boost::system
            torrent-rasterbar
            fmt
    )
else()
    target_link_libraries(${PROJECT_NAME}
            OpenSSL::SSL
            OpenSSL::Crypto
            Boost::system
            torrent-rasterbar
            fmt
    )
    # If needed for your environment:
    # target_link_libraries(${PROJECT_NAME} pthread)
endif()

#
# -------------------- Install Rules --------------------
# We install the executable on *all* platforms.
# Windows will also do fixup_bundle to gather DLLs.
#

# Common install (executable into bin/)
install(
        TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin    # For .exe or binary on all platforms
        LIBRARY DESTINATION lib    # For shared libs (non-Windows)
        ARCHIVE DESTINATION lib    # For static libs
)

# If you previously had code that installed headers, it might look like:
# install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
#         DESTINATION include
#         FILES_MATCHING
#         PATTERN "*.hpp")

# But if you *don't* want to install .hpp files, remove or comment that out.


#
# -------------------- BundleUtilities (Windows Only) --------------------
#
if(WIN32)
    # After the executable is installed, fixup_bundle will copy all needed DLLs into <prefix>/bin.
    install(CODE [[
        include(BundleUtilities)

        # Full path to the installed .exe
        set(_exe "${CMAKE_INSTALL_PREFIX}/bin/torrent_pp.exe")

        # Only do fixup_bundle if the .exe actually exists.
        if(EXISTS "${_exe}")
            message(STATUS "Running fixup_bundle on: ${_exe}")
            fixup_bundle("${_exe}" "" "")
        else()
            message(WARNING "Cannot find installed torrent_pp.exe at ${_exe}")
        endif()
    ]])
endif()
