ffmpeg_remuxer
==============

remux without recodec using ffmpeg

## Features
* remux without codec
* judge the file format according to the file extention.
* Use cmake for cross-platform build.

## Dependencies
* libavformat
* libavcodec
* libavutil

## Build
* `./start_build.sh`
* build target is located in build/ffmpeg_remuxer

## Usage
* `ffmpeg_remuxer input.{ext} output.{ext}`
