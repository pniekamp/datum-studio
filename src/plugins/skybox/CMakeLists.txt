#
# skybox plugin
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

set(FRMS ${FRMS} skyboxproperties.ui)

set(QRCS ${QRCS} skyboxplugin.qrc)

set(SRCS ${SRCS} skybox.h skybox.cpp)
set(SRCS ${SRCS} skyboxplugin.h skyboxplugin.cpp)
set(SRCS ${SRCS} skyboxeditor.h skyboxeditor.cpp)
set(SRCS ${SRCS} skyboxview.h skyboxview.cpp)
set(SRCS ${SRCS} skyboxproperties.h skyboxproperties.cpp)
set(SRCS ${SRCS} ../image/image.h ../image/image.cpp)
set(SRCS ${SRCS} ${COMMON}/viewport.h ${COMMON}/viewport.cpp)
set(SRCS ${SRCS} ${COMMON}/commandbar.h ${COMMON}/commandbar.cpp)
set(SRCS ${SRCS} ${COMMON}/droplabel.h ${COMMON}/droplabel.cpp)
set(SRCS ${SRCS} ${COMMON}/assetfile.h ${COMMON}/assetfile.cpp)
set(SRCS ${SRCS} ${COMMON}/qcslider.h ${COMMON}/qcslider.cpp)
set(SRCS ${SRCS} ${COMMON}/qccombobox.h ${COMMON}/qccombobox.cpp)
set(SRCS ${SRCS} ${COMMON}/qcdoubleslider.h ${COMMON}/qcdoubleslider.cpp)
set(SRCS ${SRCS} ${DATUM_TOOLS}/assetpacker.h ${DATUM_TOOLS}/assetpacker.cpp ${DATUM_TOOLS}/bc3.cpp ${DATUM_TOOLS}/hdr.cpp ${DATUM_TOOLS}/ibl.cpp)

qt5_wrap_ui(UIS_H ${FRMS})

qt5_add_resources(QRCS_CXX ${QRCS})

add_library(skybox SHARED ${SRCS} ${UIS_H} ${QRCS_CXX})

set_target_properties(skybox PROPERTIES COMPILE_DEFINITIONS "SKYBOXPLUGIN")

target_link_libraries(skybox datumstudio document content build datum leap vulkan Qt5::Core Qt5::Widgets)

if(WIN32)
  SET(CMAKE_SHARED_LIBRARY_PREFIX "")
endif(WIN32)

add_custom_command(TARGET skybox POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:skybox> ${CMAKE_BINARY_DIR}/bin/plugins/datumstudio/)

#
# install
#

INSTALL(TARGETS skybox DESTINATION bin/plugins/datumstudio)
