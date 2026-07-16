cmake_minimum_required(VERSION 3.16)

# Requires ESP-ADF (https://github.com/espressif/esp-adf) checked out with
# submodules and ADF_PATH exported. Tested with ADF v2.7 / IDF release-v5.3.
include($ENV{ADF_PATH}/CMakeLists.txt)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32_ipod_classic)
