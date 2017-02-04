cmake_minimum_required(VERSION 2.8)

# VitaSDK defines
if( NOT DEFINED CMAKE_TOOLCHAIN_FILE )
  if( DEFINED ENV{VITASDK} )
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(advremap)
include("${VITASDK}/share/vita.cmake" REQUIRED)
set(TITLE "advremap")
set(TITLE_ID "SHDW00001")
set(VERSION "00.10")

#set(CMAKE_C_FLAGS "-Wl,-q -O3 -g -std=c99 -Dntohs=__builtin_bswap16 -Dhtons=__builtin_bswap16 -Dntohl=__builtin_bswap32 -Dhtonl=__builtin_bswap32 -DENET_DEBUG=1")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3 -Wno-unused-variable -Wno-unused-but-set-variable -fno-lto")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1")
set(VITA_MAKE_FSELF_FLAGS "${VITA_MAKE_FSELF_FLAGS} -a 0x2800000000000001")

add_definitions(-DSQLITE_OS_OTHER=1)
add_definitions(-DSQLITE_TEMP_STORE=3)
add_definitions(-DSQLITE_THREADSAFE=0)

include_directories(
	sqlite3
)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(${PROJECT_NAME}.elf
    sqlite3/sqlite3.c
	src/vita_sqlite.c

	src/main.c
	src/guilib/lib.c
	src/guilib/ime.c

	src/ui/test_remap.c
	src/ui/main_menu.c
	src/ui/app_menu.c
	src/ui/adv_actions_menu.c

	src/remap/remap.c
	src/remap/config.c

	src/applist/applist.c
)

target_link_libraries(${PROJECT_NAME}.elf
	vita2d
	SceDisplay_stub
	SceGxm_stub
	SceSysmodule_stub
	SceCtrl_stub
	SceTouch_stub
	ScePgf_stub
	ScePower_stub
	SceAppUtil_stub

	SceCommonDialog_stub
	freetype
	png
	jpeg
	z
	m
	c
	taihen_stub

	debugnet
	SceNet_stub
	SceNetCtl_stub
	ScePower_stub
	SceAppMgr_stub
)

vita_create_self(eboot.bin ${PROJECT_NAME}.elf)
vita_create_vpk(${PROJECT_NAME}.vpk ${TITLE_ID} eboot.bin
	VERSION ${VERSION}
	NAME ${TITLE}

	FILE plugin/build/advremap.suprx advremap.suprx
)

add_custom_target(send
	COMMAND curl -T eboot.bin ftp://$(PSVITAIP):1337/ux0:/app/${TITLE_ID}/
	COMMAND curl -T ../plugin/build/advremap.suprx ftp://$(PSVITAIP):1337/ux0:/app/${TITLE_ID}/
	DEPENDS eboot.bin
)
