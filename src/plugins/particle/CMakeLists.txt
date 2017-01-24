#
# particle plugin
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

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${DATUM_TOOLS})
include_directories(${COMMON})

set(FRMS ${FRMS} particleproperties.ui)
set(FRMS ${FRMS} emitterwidget.ui)
set(FRMS ${FRMS} curveeditor.ui)

set(QRCS ${QRCS} particleplugin.qrc)

set(SRCS ${SRCS} particlesystem.h particlesystem.cpp)
set(SRCS ${SRCS} particleplugin.h particleplugin.cpp)
set(SRCS ${SRCS} particleeditor.h particleeditor.cpp)
set(SRCS ${SRCS} particleview.h particleview.cpp)
set(SRCS ${SRCS} particleproperties.h particleproperties.cpp)
set(SRCS ${SRCS} emitterlistwidget.h emitterlistwidget.cpp)
set(SRCS ${SRCS} emitterwidget.h emitterwidget.cpp)
set(SRCS ${SRCS} curveeditor.h curveeditor.cpp)
set(SRCS ${SRCS} ../image/image.h ../image/image.cpp)
set(SRCS ${SRCS} ../skybox/skybox.h ../skybox/skybox.cpp)
set(SRCS ${SRCS} ../sprite/spritesheet.h ../sprite/spritesheet.cpp)
set(SRCS ${SRCS} ${COMMON}/viewport.h ${COMMON}/viewport.cpp)
set(SRCS ${SRCS} ${COMMON}/commandbar.h ${COMMON}/commandbar.cpp)
set(SRCS ${SRCS} ${COMMON}/assetfile.h ${COMMON}/assetfile.cpp)
set(SRCS ${SRCS} ${COMMON}/droplabel.h ${COMMON}/droplabel.cpp)
set(SRCS ${SRCS} ${COMMON}/qcspinbox.h ${COMMON}/qcspinbox.cpp)
set(SRCS ${SRCS} ${COMMON}/qcdoubleslider.h ${COMMON}/qcdoubleslider.cpp)
set(SRCS ${SRCS} ${COMMON}/qcdoublespinbox.h ${COMMON}/qcdoublespinbox.cpp)
set(SRCS ${SRCS} ${DATUM_TOOLS}/assetpacker.h ${DATUM_TOOLS}/assetpacker.cpp ${DATUM_TOOLS}/bc3.cpp ${DATUM_TOOLS}/hdr.cpp ${DATUM_TOOLS}/ibl.cpp)

qt5_wrap_ui(UIS_H ${FRMS})

qt5_add_resources(QRCS_CXX ${QRCS})

add_library(particle SHARED ${SRCS} ${UIS_H} ${QRCS_CXX})

set_target_properties(particle PROPERTIES COMPILE_DEFINITIONS "PARTICLEPLUGIN")

target_link_libraries(particle datumstudio document content pack build datum leap vulkan Qt5::Core Qt5::Widgets)

if(WIN32)
  SET(CMAKE_SHARED_LIBRARY_PREFIX "")
endif(WIN32)

add_custom_command(TARGET particle POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:particle> ${CMAKE_BINARY_DIR}/bin/plugins/datumstudio/)

#
# install
#

INSTALL(TARGETS particle DESTINATION bin/plugins/datumstudio)
