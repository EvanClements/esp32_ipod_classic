cmake_minimum_required(VERSION 3.16)

# This is a standalone ESP-IDF project used only to build and run the
# track_utils unit tests on the host (Linux target). It pulls in the
# repository's components directory so it can find the code under test.
set(EXTRA_COMPONENT_DIRS "../..")

# Only build the test "main" component and whatever it depends on. Required for
# the Linux host target so ESP-IDF does not try to pull in every component.
set(COMPONENTS main)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(track_utils_host_test)
