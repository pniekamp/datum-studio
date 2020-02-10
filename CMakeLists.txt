
project(datumstudio)

cmake_minimum_required(VERSION 3.9.6)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build Type: Debug Release RelWithDebInfo MinSizeRel" FORCE)
endif(NOT CMAKE_BUILD_TYPE)

if(UNIX OR MINGW)
  set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} -latomic")
endif(UNIX OR MINGW)

if(UNIX OR MINGW)
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g" CACHE STRING "" FORCE)
endif(UNIX OR MINGW)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib CACHE INTERNAL "")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE INTERNAL "")

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR} CACHE INTERNAL "")

set(COMMON ${CMAKE_CURRENT_SOURCE_DIR}/src/common)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(Threads REQUIRED)

#
# leap
#

find_path(LEAP_INCLUDE leap.h PATHS ${PROJECT_SOURCE_DIR}/../leap/include NO_DEFAULT_PATH)
find_library(LEAP_LIBRARIES NAMES leap PATHS ${PROJECT_SOURCE_DIR}/../leap/lib)

if(LEAP_INCLUDE AND LEAP_LIBRARIES)
  add_library(leap STATIC IMPORTED)
  set_property(TARGET leap PROPERTY NO_SYSTEM_FROM_IMPORTED 1)
  set_property(TARGET leap PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${LEAP_INCLUDE})
  set_property(TARGET leap PROPERTY IMPORTED_LOCATION ${LEAP_LIBRARIES})
  set_property(TARGET leap PROPERTY INTERFACE_LINK_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})

  if(WIN32)
    set_property(TARGET leap APPEND PROPERTY INTERFACE_LINK_LIBRARIES WS2_32)
  endif(WIN32)

  message(STATUS "Found Leap: ${LEAP_INCLUDE}")
else(LEAP_INCLUDE AND LEAP_LIBRARIES)
  message(FATAL_ERROR "Could not find Leap Library")
endif(LEAP_INCLUDE AND LEAP_LIBRARIES)

#
# datum
#

find_path(DATUM_INCLUDE datum.h PATHS ${PROJECT_SOURCE_DIR}/../datum/include NO_DEFAULT_PATH)
find_path(DATUM_TOOLS assetpacker.h PATHS ${PROJECT_SOURCE_DIR}/../datum/tools NO_DEFAULT_PATH)
find_library(DATUM_LIBRARIES NAMES datum PATHS ${PROJECT_SOURCE_DIR}/../datum/lib)

if(DATUM_INCLUDE AND DATUM_LIBRARIES)
  add_library(datum STATIC IMPORTED)
  set_property(TARGET datum PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${DATUM_INCLUDE})
  set_property(TARGET datum PROPERTY IMPORTED_LOCATION ${DATUM_LIBRARIES})
  set_property(TARGET datum PROPERTY INTERFACE_LINK_LIBRARIES leap)
  message(STATUS "Found Datum: ${DATUM_INCLUDE}")
else(DATUM_INCLUDE AND DATUM_LIBRARIES)
  message(FATAL_ERROR "Could not find Datum Library")
endif(DATUM_INCLUDE AND DATUM_LIBRARIES)

#
# datum-ui
#

find_path(DATUMUIC_INCLUDE datum/ui.h PATHS ${PROJECT_SOURCE_DIR}/../datum-ui/include NO_DEFAULT_PATH)
find_library(DATUMUIC_LIBRARIES NAMES datumuic PATHS ${PROJECT_SOURCE_DIR}/../datum-ui/lib)

if(DATUMUIC_INCLUDE AND DATUMUIC_LIBRARIES)
  add_library(datumuic STATIC IMPORTED)
  set_property(TARGET datumuic PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${DATUMUIC_INCLUDE})
  set_property(TARGET datumuic PROPERTY IMPORTED_LOCATION ${DATUMUIC_LIBRARIES})
  set_property(TARGET datumuic PROPERTY INTERFACE_LINK_LIBRARIES leap)
  message(STATUS "Found DatumUi: ${DATUMUIC_INCLUDE}")
endif(DATUMUIC_INCLUDE AND DATUMUIC_LIBRARIES)

#
# vulkan
#

if(WIN32)
  find_path(VULKAN_INCLUDE vulkan/vulkan.h HINTS "$ENV{VULKAN_SDK}/Include")
  find_library(VULKAN_LIBRARIES NAMES vulkan-1 HINTS "$ENV{VULKAN_SDK}/Lib" "$ENV{VULKAN_SDK}/Bin")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
endif(WIN32)

if(UNIX)
  find_path(VULKAN_INCLUDE vulkan/vulkan.h HINTS "$ENV{VULKAN_SDK}/include")
  find_library(VULKAN_LIBRARIES NAMES libvulkan.so HINTS "$ENV{VULKAN_SDK}/lib")
  find_path(XCB_INCLUDE_DIRS xcb/xcb.h PATH_SUFFIXES xcb)
  find_library(XCB_LIBRARIES xcb)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_XCB_KHR")
endif(UNIX)

add_library(vulkan STATIC IMPORTED)
set_property(TARGET vulkan PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${VULKAN_INCLUDE})
set_property(TARGET vulkan PROPERTY IMPORTED_LOCATION ${VULKAN_LIBRARIES})

message(STATUS "Found Vulkan: ${VULKAN_INCLUDE} ${VULKAN_LIBRARIES}")

#
# Qt
#

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5X11Extras QUIET)

#
# datum-studio
#

add_subdirectory(src)
add_subdirectory(src/plugins/console)
add_subdirectory(src/plugins/project)
add_subdirectory(src/plugins/document)
add_subdirectory(src/plugins/build)
add_subdirectory(src/plugins/home)
add_subdirectory(src/plugins/pack)
add_subdirectory(src/plugins/edit)
add_subdirectory(src/plugins/content)
add_subdirectory(src/plugins/text)
add_subdirectory(src/plugins/image)
add_subdirectory(src/plugins/mesh)
add_subdirectory(src/plugins/material)
add_subdirectory(src/plugins/animation)
add_subdirectory(src/plugins/font)
add_subdirectory(src/plugins/skybox)
add_subdirectory(src/plugins/sprite)
add_subdirectory(src/plugins/model)
add_subdirectory(src/plugins/ocean)
add_subdirectory(src/plugins/terrain)
add_subdirectory(src/plugins/particle)
add_subdirectory(src/plugins/bineditor)
add_subdirectory(src/plugins/texteditor)
add_subdirectory(src/plugins/lumpimporter)
add_subdirectory(src/plugins/packimporter)
add_subdirectory(src/plugins/textimporter)
add_subdirectory(src/plugins/imgimporter)
add_subdirectory(src/plugins/hdrimporter)
add_subdirectory(src/plugins/objimporter)

#
# glslang
#

find_path(GLSLANG_ROOT_DIR lib/libglslang.dll.a HINTS "$ENV{GLSLANG_DIR}")

if(GLSLANG_ROOT_DIR)
  set(GLSLANG_INCLUDE_DIR ${GLSLANG_ROOT_DIR}/include)
  set(GLSLANG_LIBRARY_DIR ${GLSLANG_ROOT_DIR}/lib)
  set(GLSLANG_LIBRARIES glslang.dll SPIRV.dll)
  message(STATUS "Found Glslang: ${GLSLANG_ROOT_DIR}")
endif(GLSLANG_ROOT_DIR)

if(GLSLANG_ROOT_DIR)
  add_subdirectory(src/plugins/shader)
endif(GLSLANG_ROOT_DIR)

#
# assimp
#

find_package(assimp QUIET)

if(assimp_FOUND)
  message(STATUS "Found Assimp: ${ASSIMP_ROOT_DIR}")
endif(assimp_FOUND)

if(assimp_FOUND)
  add_subdirectory(src/plugins/assimporter)
endif(assimp_FOUND)

#
# DevIL
#

find_path(DEVIL_ROOT_DIR lib/libIL.dll.a HINTS "$ENV{DEVIL_DIR}")

if(DEVIL_ROOT_DIR)
  set(DEVIL_INCLUDE_DIR ${DEVIL_ROOT_DIR}/include)
  set(DEVIL_LIBRARY_DIR ${DEVIL_ROOT_DIR}/lib)
  set(DEVIL_LIBRARIES IL.dll)
  message(STATUS "Found DevIL: ${DEVIL_ROOT_DIR}")
endif(DEVIL_ROOT_DIR)

if(DEVIL_ROOT_DIR)
  add_subdirectory(src/plugins/devilimporter)
endif(DEVIL_ROOT_DIR)

#
# DatumUi
#

if(DATUMUIC_INCLUDE AND DATUMUIC_LIBRARIES)
  add_subdirectory(src/plugins/datumui)
endif(DATUMUIC_INCLUDE AND DATUMUIC_LIBRARIES)
