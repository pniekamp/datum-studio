#
# build plugin
#

set(CMAKE_CXX_STANDARD 14)

if(UNIX OR MINGW)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
endif(UNIX OR MINGW)

add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${DATUM_TOOLS})
include_directories(${COMMON})

set(SRCS ${SRCS} buildapi.h)
set(SRCS ${SRCS} buildplugin.h buildplugin.cpp)
set(SRCS ${SRCS} buildmanager.h buildmanager.cpp)
set(SRCS ${SRCS} buildstatus.h buildstatus.cpp)

add_library(build SHARED ${SRCS} ${QRCS} ${FRMS})

set_target_properties(build PROPERTIES COMPILE_DEFINITIONS "BUILDPLUGIN")

target_link_libraries(build datumstudio project document leap datum Qt5::Core Qt5::Widgets)

if(WIN32)
  SET(CMAKE_SHARED_LIBRARY_PREFIX "")
endif(WIN32)

add_custom_command(TARGET build POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:build> ${CMAKE_BINARY_DIR}/bin/plugins/datumstudio/)

#
# install
#

INSTALL(TARGETS build DESTINATION bin/plugins/datumstudio)
