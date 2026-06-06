# --------------------------------------------------------------------------
# OpenFodder build for the segrax/firy disk-image library.
#
# firy ships a Visual Studio 2019 .vcxproj only; this CMakeLists.txt is
# hand-authored so we can pull firy in via add_subdirectory() and link it as
# a static library into the openfodder executable.
#
# When firy gains its own upstream CMakeLists.txt this file should be removed
# and replaced with a forward to it. Until then, keep this file minimal and
# matched to the existing source tree.
# --------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.18)
project(firy CXX)

# firy uses std::weak_from_this, std::shared_mutex, std::filesystem.
# OpenFodder's root CMakeLists.txt already pins CMAKE_CXX_STANDARD 17.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE FIRY_SOURCES CONFIGURE_DEPENDS
    "source/*.cpp"
)
file(GLOB_RECURSE FIRY_HEADERS CONFIGURE_DEPENDS
    "source/*.hpp"
    "source/*.h"
)

add_library(firy STATIC ${FIRY_SOURCES} ${FIRY_HEADERS})

target_include_directories(firy PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/source"
)

# RTTI + exceptions are required by firy (typeid, throw std::runtime_error).
# MSVC defaults to /GR /EHsc, which is what we want — no extra flags needed.

if(MSVC)
    # firy uses Windows.h on _MSC_VER; suppress the inheritance dominance
    # warning the way firy.hpp asks (#pragma warning 4250).
    target_compile_options(firy PRIVATE /W3)
endif()

# Bring sources under the same source_group "tree" view OpenFodder uses for
# its own files, so firy shows up cleanly in Visual Studio.
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/source"
             PREFIX "firy" FILES ${FIRY_SOURCES} ${FIRY_HEADERS})
