add_executable(bull-example ${CMAKE_CURRENT_SOURCE_DIR}/compiler.cpp)
target_link_libraries(bull-example PRIVATE qdmi::qdmi qaptiva_compiler_device qdmi::project_warnings)
