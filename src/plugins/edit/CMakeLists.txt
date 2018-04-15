#
# edit plugin
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

set(FRMS ${FRMS} editplugin.ui)
set(FRMS ${FRMS} editorview.ui)

set(QRCS ${QRCS} editplugin.qrc)

set(SRCS ${SRCS} editorapi.h)
set(SRCS ${SRCS} editplugin.h editplugin.cpp)
set(SRCS ${SRCS} editorview.h editorview.cpp)
set(SRCS ${SRCS} editormanager.h editormanager.cpp)
set(SRCS ${SRCS} ${COMMON}/commandbar.h ${COMMON}/commandbar.cpp)

add_library(edit SHARED ${SRCS} ${QRCS} ${FRMS})

set_target_properties(edit PROPERTIES COMPILE_DEFINITIONS "EDITPLUGIN")

target_link_libraries(edit datumstudio project document leap Qt5::Core Qt5::Widgets)

if(WIN32)
  SET(CMAKE_SHARED_LIBRARY_PREFIX "")
endif(WIN32)

add_custom_command(TARGET edit POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:edit> ${CMAKE_BINARY_DIR}/bin/plugins/datumstudio/)

#
# install
#

INSTALL(TARGETS edit DESTINATION bin/plugins/datumstudio)
