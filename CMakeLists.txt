cmake_minimum_required(VERSION 3.23.2)

project(basilisk)

set(DIR ${CMAKE_CURRENT_LIST_DIR})

add_library(${PROJECT_NAME}
	${DIR}/src/bs_wnd.c
	${DIR}/src/bs_mem.c
	${DIR}/src/bs_ttf.c
	${DIR}/src/bs_core.c
	${DIR}/src/bs_math.c
	${DIR}/src/bs_debug.c
	${DIR}/src/bs_models.c
	${DIR}/src/bs_shaders.c
	${DIR}/src/bs_textures.c
	${DIR}/src/bs_abstract.c

	${DIR}/external/glad/glad.c
	${DIR}/external/lodepng/lodepng.c
)

include_directories(${DIR}/external/cglm)

target_include_directories(${PROJECT_NAME}
	PUBLIC ${DIR}/include
	PUBLIC ${DIR}/include/gl
	PUBLIC ${DIR}/include/basilisk
	PUBLIC ${DIR}/external/lodepng
	PUBLIC ${DIR}/external/cglm/include
)

target_link_directories(${PROJECT_NAME}
	PRIVATE ${DIR}/lib
	PRIVATE ${DIR}/external/cglm
)
