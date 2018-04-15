#
# devil importer plugin
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

set(DEVIL_LIBRARIES ${DEVIL_INSTALL}/lib/libIL.dll.a)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${DATUM_TOOLS})
include_directories(${COMMON})
include_directories(${DEVIL_INSTALL}/include)

set(QRCS ${QRCS} devilimporter.qrc)

set(SRCS ${SRCS} devilimporter.h devilimporter.cpp)
set(SRCS ${SRCS} ${COMMON}/assetfile.h ${COMMON}/assetfile.cpp)
set(SRCS ${SRCS} ${DATUM_TOOLS}/assetpacker.h ${DATUM_TOOLS}/assetpacker.cpp ${DATUM_TOOLS}/bc3.cpp)

add_library(devilimporter SHARED ${SRCS} ${QRCS} ${FRMS})

set_target_properties(devilimporter PROPERTIES COMPILE_DEFINITIONS "DEVILIMPORTERPLUGIN")

target_link_libraries(devilimporter datumstudio content leap datum Qt5::Core ${DEVIL_LIBRARIES})

if(WIN32)
  SET(CMAKE_SHARED_LIBRARY_PREFIX "")
endif(WIN32)

add_custom_command(TARGET devilimporter POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:devilimporter> ${CMAKE_BINARY_DIR}/bin/plugins/datumstudio/)

#
# install
#

INSTALL(TARGETS devilimporter DESTINATION bin/plugins/datumstudio)
