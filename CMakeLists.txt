
cmake_minimum_required (VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

#file(GLOB_RECURSE HeaderFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/**/*.h")
#file(GLOB_RECURSE CSourceFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/**/*.c")

add_library (Opal STATIC
  "include/opal_defines.h"
  "include/opal_define_buffer.h"
  "include/opal_define_framebuffer.h"
  "include/opal_define_image.h"
  "include/opal_define_material.h"
  "include/opal_define_mesh.h"
  "include/opal_define_renderpass.h"
  "include/opal_define_window.h"
  "include/opal.h"

  "src/common.h"
  "src/opal.c"
  "src/platform.c"
  "src/opal_image.c"
  "src/vulkan.c"
  "src/vulkan_buffer.c"
  "src/vulkan_command.c"
  "src/vulkan_framebuffer.c"
  "src/vulkan_image.c"
  "src/vulkan_material.c"
  "src/vulkan_render.c"
  "src/vulkan_renderpass.c"
  "src/vulkan_window.c"
)

set_target_properties(Opal PROPERTIES LINKER_LANGUAGE C)
target_compile_definitions(Opal PUBLIC OPAL_MODE=1)

target_include_directories (Opal PUBLIC
  ./
  ../Lapis/include/
  ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(Opal
  Lapis
  ${Vulkan_LIBRARIES}/../../Lib/vulkan-1.lib
)
