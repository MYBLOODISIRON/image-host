cmake_minimum_required(VERSION 3.28.3)
project(db-conn-pool LANGUAGES CXX)
add_library(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../include/)
target_sources(${PROJECT_NAME} PRIVATE db_pool.cc DBConn.cc DBManager.cc PrepareStatement.cc ResultSet.cc AutoRelConn.cc)
target_link_libraries(${PROJECT_NAME} PUBLIC mysqlclient)