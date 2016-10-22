#
# imgviewer plugin
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

set(FRMS ${FRMS} imgproperties.ui)

set(QRCS ${QRCS} imageplugin.qrc)

set(SRCS ${SRCS} image.h image.cpp)
set(SRCS ${SRCS} imageplugin.h imageplugin.cpp)
set(SRCS ${SRCS} imgviewer.h imgviewer.cpp)
set(SRCS ${SRCS} imgview.h imgview.cpp)
set(SRCS ${SRCS} imgproperties.h imgproperties.cpp)
set(SRCS ${SRCS} ${COMMON}/viewport.h ${COMMON}/viewport.cpp)
set(SRCS ${SRCS} ${COMMON}/commandbar.h ${COMMON}/commandbar.cpp)
set(SRCS ${SRCS} ${COMMON}/assetfile.h ${COMMON}/assetfile.cpp)
set(SRCS ${SRCS} ${COMMON}/qcslider.h ${COMMON}/qcslider.cpp)
set(SRCS ${SRCS} ${COMMON}/qcdoubleslider.h ${COMMON}/qcdoubleslider.cpp)
set(SRCS ${SRCS} ${COMMON}/qcfilelineedit.h ${COMMON}/qcfilelineedit.cpp)
set(SRCS ${SRCS} ${DATUM_TOOLS}/assetpacker.h ${DATUM_TOOLS}/assetpacker.cpp ${DATUM_TOOLS}/bc3.cpp)

qt5_wrap_ui(UIS_H ${FRMS})

qt5_add_resources(QRCS_CXX ${QRCS})

add_library(imgviewer SHARED ${SRCS} ${UIS_H} ${QRCS_CXX})

set_target_properties(imgviewer PROPERTIES COMPILE_DEFINITIONS "IMGVIEWERPLUGIN")

target_link_libraries(imgviewer datumstudio document content datum leap vulkan Qt5::Core Qt5::Widgets)

if(WIN32)
  SET(CMAKE_SHARED_LIBRARY_PREFIX "")
endif(WIN32)

add_custom_command(TARGET imgviewer POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:imgviewer> ${CMAKE_BINARY_DIR}/bin/plugins/datumstudio/)

#
# install
#

INSTALL(TARGETS imgviewer DESTINATION bin/plugins/datumstudio)
