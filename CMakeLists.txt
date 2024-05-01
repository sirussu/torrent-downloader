cmake_minimum_required(VERSION 3.15)

project(torrent_pp)

set(OUTPUT_DIR "win32-x64")

set(PLATFORM_DEFINES
    -D_WIN32_WINNT=0x0601
    -DWIN32)
    
set(BOOST_DEBUG ON)

if(DEFINED ENV{BOOST_ROOT})
    set(BOOST_ROOT $ENV{BOOST_ROOT})
endif()
if (NOT DEFINED BOOST_ROOT)
    if(DEFINED $ENV{BOOST_ROOT})
        set(BOOST_ROOT $ENV{BOOST_ROOT})
    endif()
endif()

if (DEFINED BOOST_ROOT)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.0)
        set(BOOST_LIBRARYDIR ${BOOST_ROOT}/lib${PLATFORM}-msvc-12.0)
    else()
        if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.10)
            set(BOOST_LIBRARYDIR ${BOOST_ROOT}/lib${PLATFORM}-msvc-14.0)
        else()
            if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.20)
                list(APPEND BOOST_LIBRARYDIR
                    ${BOOST_ROOT}/lib${PLATFORM}-msvc-14.1
                    ${BOOST_ROOT}/lib${PLATFORM}-msvc-14.0 )
            else()
                list(APPEND BOOST_LIBRARYDIR
                    ${BOOST_ROOT}/lib${PLATFORM}-msvc-14.2
                    ${BOOST_ROOT}/lib${PLATFORM}-msvc-14.1 )
            endif()
        endif()
    endif()
else()
    message(FATAL_ERROR "No BOOST_ROOT environment variable could be found! Please make sure it is set and the points to your Boost installation.")
endif()

set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     OFF)

set(BOOST_REQUIRED_VERSION 1.68)

find_package(Boost ${BOOST_REQUIRED_VERSION} REQUIRED system)
include_directories(${Boost_INCLUDE_DIRS})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(BUILD_SHARED_LIBS       OFF)

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

set(deprecated-functions OFF)
set(static_runtime ON)

add_subdirectory(libtorrent)
add_subdirectory(fmt)

file(GLOB SOURCE_FILES "src/*")

add_executable(
    ${PROJECT_NAME}
    ${SOURCE_FILES})

set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL
        CMAKE_CXX_FLAGS_RELWITHDEBINFO
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL
        CMAKE_C_FLAGS_RELWITHDEBINFO
        )
foreach(CompilerFlag ${CompilerFlags})
    string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    set(${CompilerFlag} "${${CompilerFlag}}" CACHE STRING "msvc compiler flags" FORCE)
    message("MSVC flags: ${CompilerFlag}:${${CompilerFlag}}")
endforeach()

set_property(TARGET torrent_pp PROPERTY CXX_STANDARD 17)
target_compile_features(torrent_pp
	PUBLIC
		cxx_std_17)
    
target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    "${CMAKE_SOURCE_DIR}/libtorrent/include")

# check if we are on windows
if (WIN32)
target_link_libraries(
    ${PROJECT_NAME} 
    crypt32
    OpenSSL::SSL
    OpenSSL::Crypto
    Boost::system 
    "torrent-rasterbar" 
    "fmt" )
else()
target_link_libraries(
    ${PROJECT_NAME} 
    OpenSSL::SSL
    OpenSSL::Crypto
    Boost::system 
    "torrent-rasterbar" 
    "fmt" )
endif()
