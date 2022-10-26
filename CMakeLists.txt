cmake_minimum_required(VERSION 3.23.2)

project(basilisk)

add_library(${PROJECT_NAME}
	src/bs_wnd.c
	src/bs_mem.c
	src/bs_ttf.c
	src/bs_core.c
	src/bs_math.c
	src/bs_debug.c
	src/bs_models.c
	src/bs_shaders.c
	src/bs_textures.c
	src/bs_abstract.c

	external/glad/glad.c
	external/lodepng/lodepng.c
)

add_subdirectory(external/cglm)

target_include_directories(${PROJECT_NAME}
	PUBLIC include
	PUBLIC include/gl
	PUBLIC include/basilisk
	PUBLIC external/lodepng
	PUBLIC external/cglm/include
)

target_link_directories(${PROJECT_NAME}
	PRIVATE lib
	PRIVATE external/cglm/src
)

target_link_libraries(${PROJECT_NAME}
	cglm
)
