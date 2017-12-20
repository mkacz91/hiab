cmake_minimum_required(VERSION 3.4.1 FATAL_ERROR)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 11)

project(Hiab)

find_package(OpenGL REQUIRED)
find_package(GLFW3 REQUIRED)

set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

set(GLAD_DIR "${CMAKE_BINARY_DIR}/glad")
set(GLAD_SOURCES "${GLAD_DIR}/glad.c" "${GLAD_DIR}/glad.h")
add_custom_command(
  OUTPUT ${GLAD_SOURCES}
  COMMAND python -m glad --api gl=3.2 --profile core --extensions "${SRC_DIR}/glext.txt" --generator c --no-loader --omit-khrplatform --local-files --out-path "${GLAD_DIR}"
)

add_library(glad ${GLAD_SOURCES})

if (WIN32)
  add_definitions(-DHIAB_WINDOWS)
endif (WIN32)

include_directories(
  ${GLAD_DIR}
  ${GLFW3_INCLUDE_DIR}
)

file(GLOB HIAB_SOURCES src/*.cpp src/*.h)
add_executable(hiab ${HIAB_SOURCES})
target_link_libraries(hiab
  glfw3
  glad
  ${OPENGL_gl_LIBRARY}
)

if (WIN32)
  get_property(GLFW3_DLL TARGET glfw3 PROPERTY IMPORTED_LOCATION)
  add_custom_command(TARGET hiab POST_BUILD COMMAND
  ${CMAKE_COMMAND} -E copy_if_different "${GLFW3_DLL}" "$<TARGET_FILE_DIR:hiab>")
endif (WIN32)
