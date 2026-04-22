cmake_minimum_required(VERSION 3.28.3)
project(cache-pool LANGUAGES CXX)
add_library(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_sources(${PROJECT_NAME} PRIVATE cache_pool.cc)

include(FetchContent)
FetchContent_Declare(MyReactor SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../my-reactor)
FetchContent_MakeAvailable(MyReactor)

target_link_libraries(${PROJECT_NAME} PUBLIC hiredis MyReactor)