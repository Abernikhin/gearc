add_subdirectory(lexer ${CMAKE_CACHE_OUTPUT_DIRECTORIES}/lexer)
add_subdirectory(parser ${CMAKE_CACHE_OUTPUT_DIRECTORIES}/parser)
add_subdirectory(MIR ${CMAKE_CACHE_OUTPUT_DIRECTORIES}/MIR)

add_executable(gearc main.c)
target_link_libraries(gearc PUBLIC lexer parser semantic MIR)