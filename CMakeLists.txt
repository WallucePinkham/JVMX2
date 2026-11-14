cmake_minimum_required(VERSION 3.16)

project(JVMX2 LANGUAGES C CXX)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message(FATAL_ERROR "This build definition currently targets Linux only.")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost 1.78.0 EXACT REQUIRED COMPONENTS program_options thread)
find_package(libffi CONFIG REQUIRED)
find_package(Threads REQUIRED)

file(GLOB JVMX2_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/JVMX2/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/JVMX2/Zip/*.cpp"
)
list(FILTER JVMX2_SOURCES EXCLUDE REGEX "/OperatingSystemWindows\\.cpp$")

file(GLOB ZLIB_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/zlib/*.c"
)

add_library(jvmx2_zlib STATIC ${ZLIB_SOURCES})
target_include_directories(jvmx2_zlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/zlib")

add_executable(JVMX2 ${JVMX2_SOURCES})
target_include_directories(JVMX2 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/wallaroo")
target_compile_options(JVMX2 PRIVATE
  "SHELL:-iquote \"${CMAKE_CURRENT_SOURCE_DIR}/JVMX2\""
  "SHELL:-iquote \"${CMAKE_CURRENT_SOURCE_DIR}/JVMX2/Zip\""
)
target_link_libraries(JVMX2 PRIVATE
  Boost::program_options
  Boost::thread
  libffi
  Threads::Threads
  ${CMAKE_DL_LIBS}
  jvmx2_zlib
)
