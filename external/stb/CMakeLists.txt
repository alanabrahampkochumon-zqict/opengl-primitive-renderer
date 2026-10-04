add_library(stb INTERFACE stb_image.h)
target_compile_definitions(stb INTERFACE STB_IMAGE_IMPLEMENTATION)
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_LIST_DIR})