#
# project plugin
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
include_directories(${COMMON})

set(FRMS ${FRMS} newproject.ui)

set(QRCS ${QRCS} projectplugin.qrc)

set(SRCS ${SRCS} projectapi.h)
set(SRCS ${SRCS} projectplugin.h projectplugin.cpp)
set(SRCS ${SRCS} projectmanager.h projectmanager.cpp)
set(SRCS ${SRCS} ${COMMON}/dialogfactory.h)
set(SRCS ${SRCS} ${COMMON}/qcfilelineedit.h ${COMMON}/qcfilelineedit.cpp)

add_library(project SHARED ${SRCS} ${QRCS} ${FRMS})

set_target_properties(project PROPERTIES COMPILE_DEFINITIONS "PROJECTPLUGIN")

target_link_libraries(project datumstudio leap datum Qt5::Core Qt5::Widgets)

if(WIN32)
  SET(CMAKE_SHARED_LIBRARY_PREFIX "")
endif(WIN32)

add_custom_command(TARGET project POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:project> ${CMAKE_BINARY_DIR}/bin/plugins/datumstudio/)

#
# install
#

INSTALL(TARGETS project DESTINATION bin/plugins/datumstudio)
