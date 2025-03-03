include_directories(
  ${CMAKE_SOURCE_DIR}/libs/bench_lib/inc
  ${CMAKE_SOURCE_DIR}/libs/stdio_lib/inc
  ${CMAKE_SOURCE_DIR}/libs/string_lib/inc
  ${CMAKE_SOURCE_DIR}/libs/sys_lib/inc
  .
)

add_subdirectory(HV_binding)
add_subdirectory(HV_bundling)
add_subdirectory(HV_similarity)
add_subdirectory(HV_clipping)
add_subdirectory(HV_permutation)
add_subdirectory(HV_search)
add_subdirectory(HDCU_benchmark)
