#
# pack plugin
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

set(FRMS ${FRMS} build.ui)
set(FRMS ${FRMS} properties.ui)
set(FRMS ${FRMS} packplugin.ui)

set(QRCS ${QRCS} packplugin.qrc)

set(SRCS ${SRCS} packapi.h)
set(SRCS ${SRCS} packplugin.h packplugin.cpp)
set(SRCS ${SRCS} packmodel.h packmodel.cpp)
set(SRCS ${SRCS} treeview.h treeview.cpp)
set(SRCS ${SRCS} fileview.h fileview.cpp)
set(SRCS ${SRCS} pack.h pack.cpp)
set(SRCS ${SRCS} ${COMMON}/commandbar.h ${COMMON}/commandbar.cpp)
set(SRCS ${SRCS} ${COMMON}/assetfile.h ${COMMON}/assetfile.cpp)
set(SRCS ${SRCS} ${DATUM_TOOLS}/assetpacker.h ${DATUM_TOOLS}/assetpacker.cpp ${DATUM_TOOLS}/bc3.cpp)

add_library(pack SHARED ${SRCS} ${QRCS} ${FRMS})

set_target_properties(pack PROPERTIES COMPILE_DEFINITIONS "PACKPLUGIN")

target_link_libraries(pack datumstudio project document build edit content leap Qt5::Core Qt5::Widgets)

if(WIN32)
  SET(CMAKE_SHARED_LIBRARY_PREFIX "")
endif(WIN32)

add_custom_command(TARGET pack POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pack> ${CMAKE_BINARY_DIR}/bin/plugins/datumstudio/)

#
# install
#

INSTALL(TARGETS pack DESTINATION bin/plugins/datumstudio)
