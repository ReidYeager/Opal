
cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

add_library (Opal STATIC
  "include/opal.h"
  "include/opal_defines.h"

  "src/defines.h"
  "src/core.c"

  "src/vulkan/vulkan.h"
  "src/vulkan/vulkan_defines.h"
  "src/vulkan/core.c"
  "src/vulkan/material.c" "src/vulkan/buffer.c" "src/buffer.c" "src/material.c" "src/vulkan/command.c" "src/mesh.c" "src/vulkan/mesh.c")

set_target_properties(Opal PROPERTIES LINKER_LANGUAGE C)

target_include_directories (Opal PUBLIC
  ./
  ${CMAKE_SOURCE_DIR}/Lapis/include/
  ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(Opal
  Lapis
  Vulkan::Vulkan
)
