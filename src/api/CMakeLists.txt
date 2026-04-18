cmake_minimum_required(VERSION 3.28.3)
include_directories(./include)
aux_source_directory(./ SRC_LIST)
add_library(api SHARED ${SRC_LIST})