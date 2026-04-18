cmake_minimum_requeired(VERSION 3.28.3)
include_directories(./include)
aux_source_directory(./ SRC_LIST)
add_library(api ${SRC_LIST})