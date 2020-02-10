#
# shader plugin
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
include_directories(${GLSLANG_INCLUDE_DIR})

link_directories(${GLSLANG_LIBRARY_DIR})

set(QRCS ${QRCS} shaderplugin.qrc)

set(SRCS ${SRCS} shaderplugin.h shaderplugin.cpp)
set(SRCS ${SRCS} ${COMMON}/assetfile.h ${COMMON}/assetfile.cpp)
set(SRCS ${SRCS} ${DATUM_TOOLS}/assetpacker.h ${DATUM_TOOLS}/assetpacker.cpp ${DATUM_TOOLS}/bc3.cpp)

add_library(shader SHARED ${SRCS} ${QRCS} ${FRMS})

set_target_properties(shader PROPERTIES COMPILE_DEFINITIONS "SHADERPLUGIN")

target_link_libraries(shader datumstudio document content pack build leap ${GLSLANG_LIBRARIES} Qt5::Core Qt5::Widgets)

if(WIN32)
  SET(CMAKE_SHARED_LIBRARY_PREFIX "")
endif(WIN32)

add_custom_command(TARGET shader POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:shader> ${CMAKE_BINARY_DIR}/bin/plugins/datumstudio/)

#
# install
#

INSTALL(TARGETS shader DESTINATION bin/plugins/datumstudio)
