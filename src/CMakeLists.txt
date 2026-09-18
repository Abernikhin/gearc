add_subdirectory(lexer ${CMAKE_CACHE_OUTPUT_DIRECTORIES}/lexer)

add_executable(gearc main.c)
target_link_libraries(gearc PUBLIC lexer)