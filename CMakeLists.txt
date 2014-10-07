#
# @file CMakeLists.txt
# @author Akagi201
# @date 2014/10/07
#

cmake_minimum_required(VERSION 2.8.4)
project(ffmpeg_remuxer)

set(SOURCE_FILES main.c)

include_directories("/usr/local/include")

link_directories("/usr/local/lib")

add_executable(ffmpeg_remuxer ${SOURCE_FILES})

target_link_libraries(ffmpeg_remuxer "libavformat.a")

target_link_libraries(ffmpeg_remuxer "libavcodec.a")

target_link_libraries(ffmpeg_remuxer "libavutil.a")
