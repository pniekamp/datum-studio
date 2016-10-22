#
# datum-studio
#

set(CMAKE_CXX_STANDARD 14)

if(UNIX OR MINGW)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
endif(UNIX OR MINGW)

add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)

set(CMAKE_AUTOMOC ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${COMMON})

set(FRMS ${FRMS} forms/datumstudio.ui)
set(FRMS ${FRMS} forms/about.ui)

set(QRCS ${QRCS} forms/resources.qrc)

set(SRCS ${SRCS} main.cpp)
set(SRCS ${SRCS} api.h)
set(SRCS ${SRCS} core.h core.cpp)
set(SRCS ${SRCS} metabar.h metabar.cpp)
set(SRCS ${SRCS} statusbar.h statusbar.cpp)
set(SRCS ${SRCS} datumstudio.h datumstudio.cpp)
set(SRCS ${SRCS} platform.h platform.cpp)
set(SRCS ${SRCS} common/commandbar.h common/commandbar.cpp)

qt5_wrap_ui(UIS_H ${FRMS})

qt5_add_resources(QRCS_CXX ${QRCS})

add_executable(datumstudio ${SRCS} ${UIS_H} ${QRCS_CXX})

set_target_properties(datumstudio PROPERTIES ENABLE_EXPORTS on)
set_target_properties(datumstudio PROPERTIES COMPILE_DEFINITIONS "DATUMSTUDIO")

target_link_libraries(datumstudio datum leap vulkan Qt5::Core Qt5::Widgets)

if(WIN32)
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,-subsystem,windows")
endif(WIN32)

add_custom_command(TARGET datumstudio PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/plugins/datumstudio/)

#
# install
#

INSTALL(TARGETS datumstudio DESTINATION bin)

if(WIN32)
  INSTALL(FILES ${EXECUTABLE_OUTPUT_PATH}/libdatumstudio.dll.a DESTINATION lib)
endif(WIN32)
