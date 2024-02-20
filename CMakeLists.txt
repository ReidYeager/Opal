
cmake_minimum_required (VERSION 3.20)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE HeaderFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/**.h")
file(GLOB_RECURSE CSourceFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/**.c")

message(WARNING "Opal sources: ${CSourceFiles}")

add_library (opal STATIC
  ${HeaderFiles}
  ${CSourceFiles}
)

set_target_properties(opal PROPERTIES LINKER_LANGUAGE C)
target_compile_definitions(opal PUBLIC "GEM_OPAL")

target_include_directories (opal PUBLIC
  ./
  ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(opal
  ${Vulkan_LIBRARIES}/../../Lib/vulkan-1.lib
)
