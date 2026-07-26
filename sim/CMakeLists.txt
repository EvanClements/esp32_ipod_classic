cmake_minimum_required(VERSION 3.16)
project(ipod_ui_sim C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FetchContent)

# Point LVGL at our headless configuration and skip its examples/demos so the
# build stays small and fast.
set(LV_CONF_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h CACHE STRING "" FORCE)
set(LV_CONF_BUILD_DISABLE_EXAMPLES ON CACHE BOOL "" FORCE)
set(LV_CONF_BUILD_DISABLE_DEMOS ON CACHE BOOL "" FORCE)

FetchContent_Declare(
  lvgl
  GIT_REPOSITORY https://github.com/lvgl/lvgl.git
  GIT_TAG v9.2.2
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(lvgl)

add_executable(ipod_ui_sim
  main.c
  frame_png.c
  tour.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../ui/ui.c
)

target_include_directories(ipod_ui_sim PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../ui
)

target_link_libraries(ipod_ui_sim PRIVATE lvgl)
