
cmake_minimum_required (VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

add_library (Opal STATIC
  "include/opal.h"
  "include/opal_defines.h"

  "src/defines.h"
  "src/common.h"
  "src/buffer.c"
  "src/core.c"
  "src/image.c"
  "src/mesh.c"
  "src/material.c"
  "src/renderpass.c"

  "src/vulkan/vulkan_common.h"
  "src/vulkan/vulkan_defines.h"
  "src/vulkan/buffer.c"
  "src/vulkan/command.c"
  "src/vulkan/core.c"
  "src/vulkan/image.c"
  "src/vulkan/material.c"
  "src/vulkan/rendering.c"
  "src/vulkan/mesh.c"
)

set_target_properties(Opal PROPERTIES LINKER_LANGUAGE C)

target_include_directories (Opal PUBLIC
  ./
  ../Lapis/include/
  ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(Opal
  Lapis
  Vulkan::Vulkan
)
