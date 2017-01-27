cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(advremap)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -std=gnu99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")

include_directories(
)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

if (NOT ${RELEASE})
  add_definitions(-DENABLE_LOGGING)
endif()

add_executable(advremap
  main.c
  font.c
  blit.c
  ../src/remap/remap.c
  ../src/remap/config.c
)

add_definitions( -DPLUGIN=1 )

target_link_libraries(advremap
  taihen_stub
  SceLibc_stub
  SceLibKernel_stub
  SceRtc_stub

  SceCtrl_stub
  SceTouch_stub
  SceDisplay_stub
)

set_target_properties(advremap
  PROPERTIES LINK_FLAGS "-nostdlib"
)

vita_create_self(advremap.suprx advremap
  UNCOMPRESSED
  CONFIG ${CMAKE_SOURCE_DIR}/main.yml
)

add_custom_target(debug_send
  COMMAND dd if=../debug_config of=advremap.suprx bs=1 seek=70324 conv=notrunc
  COMMAND curl -T advremap.suprx ftp://$(PSVITAIP):1337/ux0:/tai/advremap/debug.suprx
  DEPENDS advremap.suprx
)

add_custom_target(addr 
  COMMAND strings -o advremap.suprx | grep ADVREMAP_IM_A_LOUSY_PROGRAMMER
  DEPENDS advremap.suprx
)
