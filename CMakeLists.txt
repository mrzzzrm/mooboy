cmake_minimum_required(VERSION 2.8.9 FATAL_ERROR)

set(META_PROJECT_NAME "mooboy")
set(META_VERSION_MAJOR "0")
set(META_VERSION_MINOR "2")
set(META_VERSION_PATCH "0")

set(EXEC_NAME mooboy)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

project(mooboy C)

find_package(SDL REQUIRED)
find_package(SDL_ttf REQUIRED)
find_package(SDL_image REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(${EXEC_NAME}
    src/sys/sdl/input.h
    src/sys/sdl/serial.c
    src/sys/sdl/video.h
    src/sys/sdl/video.c
    src/sys/sdl/audio.c
    src/sys/sdl/input.c
    src/sys/sdl/audio.h
    src/sys/sdl/sdl.c
    src/sys/sys.h

    src/core/maps.h
    src/core/serial.c
    src/core/load.h
    src/core/joy.c
    src/core/io.h
    src/core/maps.c
    src/core/mem.c
    src/core/lcd.h
    src/core/ints.h
    src/core/sound.c
    src/core/hw.c
    src/core/cpu.c
    src/core/defines.h
    src/core/moo.h
    src/core/rtc.c
    src/core/lcd.c
    src/core/obj.h
    src/core/moo.c
    src/core/mbc.c
    src/core/mbc.h
    src/core/serial.h
    src/core/timers.h
    src/core/obj.c
    src/core/ops.c
    src/core/cpu.h
    src/core/hw.h
    src/core/ops.h
    src/core/sound.h
    src/core/joy.h
    src/core/load.c
    src/core/timers.c
    src/core/mem.h
    src/core/io.c
    src/core/rtc.h
    src/core/ints.c

    src/main.c
    src/menu/sdl/rom.c
    src/menu/sdl/util.c
    src/menu/sdl/dialogs.h
    src/menu/sdl/options.c
    src/menu/sdl/options.h
    src/menu/sdl/menu.c
    src/menu/sdl/rom.h
    src/menu/sdl/dialog.c
    src/menu/sdl/dialogs.c
    src/menu/sdl/dialog.h
    src/menu/sdl/util.h
    src/menu/menu.h
    
    src/debug/disasm.c
    src/debug/disasm.h
    src/debug/event.c
    src/debug/event.h
    src/debug/record.c
    src/debug/record.h

    src/util/last_rom.h
    src/util/card.c
    src/util/performance.h
    src/util/config.c
    src/util/state.h
    src/util/pathes.h
    src/util/last_rom.c
    src/util/framerate.c
    src/util/state.c
    src/util/config.h
    src/util/pathes.c
    src/util/performance.c
    src/util/speed.c
    src/util/speed.h
    src/util/card.h
    src/util/framerate.h
)

target_link_libraries(${EXEC_NAME}
    ${SDL_LIBRARY}
    ${SDLTTF_LIBRARY}
    ${SDLIMAGE_LIBRARY}
    SDL_gfx
)
