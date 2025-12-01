cmake_minimum_required(VERSION 3.10)
project(AttendanceManagementSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find MySQL
find_path(MYSQL_INCLUDE_DIR mysql/mysql.h
    PATHS
    /usr/include
    /usr/local/include
    /opt/homebrew/include
)

find_library(MYSQL_LIBRARY
    NAMES mysqlclient
    PATHS
    /usr/lib
    /usr/local/lib
    /opt/homebrew/lib
)

if(NOT MYSQL_INCLUDE_DIR OR NOT MYSQL_LIBRARY)
    message(FATAL_ERROR "MySQL library not found! Please install MySQL development files.")
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${MYSQL_INCLUDE_DIR}
)

# Source files
set(SOURCES
    src/main.cpp
    src/Database.cpp
    src/UIHelper.cpp
    src/Config.cpp
    src/BaseController.cpp
    src/AdminController.cpp
    src/TeacherController.cpp
    src/StudentController.cpp
)

add_executable(attendance_system ${SOURCES})

target_link_libraries(attendance_system ${MYSQL_LIBRARY})

# Installation
install(TARGETS attendance_system DESTINATION bin)
