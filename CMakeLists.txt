cmake_minimum_required(VERSION 2.8)

set(CMAKE_SYSTEM_NAME "Generic")
set(CMAKE_C_COMPILER "arm-vita-eabi-gcc")
set(CMAKE_CXX_COMPILER "arm-vita-eabi-g++")

project(advbuttonremap)
set(TITLE_ID "SHDW00001")

set(CMAKE_C_FLAGS "-Wl,-q -O3 -g -std=c99 -Dntohs=__builtin_bswap16 -Dhtons=__builtin_bswap16 -Dntohl=__builtin_bswap32 -Dhtonl=__builtin_bswap32 -DENET_DEBUG=1")

include_directories(
)

add_executable(${PROJECT_NAME}.elf
	src/main.c
	src/guilib/lib.c
	src/ui/test_remap.c
	src/ui/main_menu.c
	src/ui/app_menu.c

	src/remap/remap.c
	src/remap/config.c
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

	SceCommonDialog_stub
	freetype 
	png 
	jpeg 
	z 
	m
	c

	debugnet 
	SceSysmodule_stub
	SceNet_stub 
	SceNetCtl_stub 
	ScePower_stub 
	SceAppMgr_stub
)

add_custom_target(${PROJECT_NAME}.vpk ALL
	COMMAND vita-elf-create ${PROJECT_NAME}.elf ${PROJECT_NAME}.velf
	COMMAND vita-make-fself -s ${PROJECT_NAME}.velf eboot.bin
	COMMAND vita-mksfoex -s TITLE_ID=${TITLE_ID} "${PROJECT_NAME}" param.sfo
	COMMAND vita-pack-vpk -s param.sfo -b eboot.bin ${PROJECT_NAME}.vpk
)
add_dependencies(${PROJECT_NAME}.vpk ${PROJECT_NAME}.elf)
