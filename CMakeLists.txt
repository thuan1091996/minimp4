cmake_minimum_required(VERSION 3.20)

project(mp4_gen VERSION 1.0.0)



#Build final executable
add_executable(mp4_gen mp4recorder.c)

include(FetchContent)
fetchcontent_Declare(
  sndfile
  URL https://github.com/libsndfile/libsndfile/releases/download/1.2.0/libsndfile-1.2.0.tar.xz
)

fetchcontent_Declare(
  opus
  URL https://github.com/xiph/opus/releases/download/v1.4/opus-1.4.tar.gz
)
fetchcontent_MakeAvailable(opus sndfile)

# Build library fdk-aac
# message(STATUS "Including directory:  ${CMAKE_CURRENT_SOURCE_DIR}/fdk-aac")
file(CREATE_LINK ${Opus_SOURCE_DIR}/include ${Opus_SOURCE_DIR}/include/opus SYMBOLIC)
include_directories(${Opus_SOURCE_DIR}/include ${sndfile_SOURCE_DIR}/include)
target_include_directories(mp4_gen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/fdk-aac )



set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Try 2 - Use import target and import location
ADD_LIBRARY(fdk-aac STATIC IMPORTED)
SET_TARGET_PROPERTIES(fdk-aac PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/fdk-aac/libfdk-aac.a)
target_link_libraries(fdk-aac INTERFACE m)
TARGET_LINK_LIBRARIES(mp4_gen PUBLIC fdk-aac)



# Build library g711
add_library(g711
            g711.c )

# Link mp4_gen with libraries
target_link_libraries(mp4_gen PUBLIC g711 sndfile)  