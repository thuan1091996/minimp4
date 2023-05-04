#!/bin/sh
echo "Preparing input files"
mkdir build
cp video_event.h264 ./build/raw.h264
cp stream.pcm ./build/output.g711

echo "Running CMake"
cd build && cmake .. && make

echo "Running program"
chmod +x ./mp4_gen && ./mp4_gen