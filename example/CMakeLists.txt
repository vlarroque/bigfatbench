project(bfb_examples)

add_executable(bfb_example example.cpp)
target_link_libraries(bfb_example PRIVATE bfb)
target_compile_features(bfb_example PRIVATE cxx_std_11)
target_compile_options(bfb_example PRIVATE -Wall -Wextra -Wpedantic)