cmake_minimum_required(VERSION 3.23.2)

project(basilisk)

add_library(${PROJECT_NAME}
	src/basilisk/bs_core.c
	src/basilisk/bs_math.c
	src/basilisk/bs_debug.c
	src/basilisk/bs_models.c
	src/basilisk/bs_shaders.c
	src/basilisk/bs_textures.c
	src/basilisk/bs_file_mgmt.c

	external/glad/glad.c
	external/lodepng/lodepng.c
)

add_subdirectory(external/glfw)
add_subdirectory(external/cglm)

target_include_directories(${PROJECT_NAME}
	PUBLIC include
	PUBLIC include/gl
	PUBLIC include/basilisk
	PUBLIC external/lodepng
	PUBLIC external/glfw/include
	PUBLIC external/cglm/include
)

target_link_directories(${PROJECT_NAME}
	PRIVATE lib
	PRIVATE external/glfw/src
	PRIVATE external/cglm/src
)

target_link_libraries(${PROJECT_NAME}
	cglm
	glfw
)