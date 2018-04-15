#
# model plugin
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

set(FRMS ${FRMS} modelproperties.ui)
set(FRMS ${FRMS} materialwidget.ui)

set(QRCS ${QRCS} modelplugin.qrc)

set(SRCS ${SRCS} model.h model.cpp)
set(SRCS ${SRCS} modelplugin.h modelplugin.cpp)
set(SRCS ${SRCS} modeleditor.h modeleditor.cpp)
set(SRCS ${SRCS} modelview.h modelview.cpp)
set(SRCS ${SRCS} modelproperties.h modelproperties.cpp)
set(SRCS ${SRCS} meshlistwidget.h meshlistwidget.cpp)
set(SRCS ${SRCS} slotlistwidget.h slotlistwidget.cpp)
set(SRCS ${SRCS} materialwidget.h materialwidget.cpp)
set(SRCS ${SRCS} ../image/image.h ../image/image.cpp)
set(SRCS ${SRCS} ../mesh/mesh.h ../mesh/mesh.cpp)
set(SRCS ${SRCS} ../skybox/skybox.h ../skybox/skybox.cpp)
set(SRCS ${SRCS} ../material/material.h ../material/material.cpp)
set(SRCS ${SRCS} ${COMMON}/viewport.h ${COMMON}/viewport.cpp)
set(SRCS ${SRCS} ${COMMON}/commandbar.h ${COMMON}/commandbar.cpp)
set(SRCS ${SRCS} ${COMMON}/assetfile.h ${COMMON}/assetfile.cpp)
set(SRCS ${SRCS} ${COMMON}/droplabel.h ${COMMON}/droplabel.cpp)
set(SRCS ${SRCS} ${COMMON}/qcslider.h ${COMMON}/qcslider.cpp)
set(SRCS ${SRCS} ${COMMON}/qcdoubleslider.h ${COMMON}/qcdoubleslider.cpp)
set(SRCS ${SRCS} ${COMMON}/qcdoublespinbox.h ${COMMON}/qcdoublespinbox.cpp)
set(SRCS ${SRCS} ${COMMON}/qcfilelineedit.h ${COMMON}/qcfilelineedit.cpp)
set(SRCS ${SRCS} ${DATUM_TOOLS}/assetpacker.h ${DATUM_TOOLS}/assetpacker.cpp ${DATUM_TOOLS}/bc3.cpp ${DATUM_TOOLS}/hdr.cpp ${DATUM_TOOLS}/ibl.cpp)

add_library(model SHARED ${SRCS} ${QRCS} ${FRMS})

set_target_properties(model PROPERTIES COMPILE_DEFINITIONS "MODELPLUGIN")

target_link_libraries(model datumstudio document content pack build datum leap vulkan Qt5::Core Qt5::Widgets)

if(WIN32)
  SET(CMAKE_SHARED_LIBRARY_PREFIX "")
endif(WIN32)

add_custom_command(TARGET model POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:model> ${CMAKE_BINARY_DIR}/bin/plugins/datumstudio/)

#
# install
#

INSTALL(TARGETS model DESTINATION bin/plugins/datumstudio)
