
cmake_minimum_required (VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE HeaderFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/**/*.h")
file(GLOB_RECURSE CSourceFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/**/*.c")

add_library (Opal STATIC
  ${HeaderFiles}
  ${CSourceFiles}
)

set_target_properties(Opal PROPERTIES LINKER_LANGUAGE C)

target_include_directories (Opal PUBLIC
  ./
  ../Lapis/include/
  ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(Opal
  Lapis
  ${Vulkan_LIBRARIES}/../../Lib/vulkan-1.lib
)
