cmake_minimum_required (VERSION 3.1.0)

project (icfpc2015)

add_executable(play_icfp2015 main.cpp jsoncpp.cpp)

target_compile_features(play_icfp2015 PRIVATE cxx_range_for)
