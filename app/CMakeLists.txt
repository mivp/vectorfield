set_property(GLOBAL PROPERTY USE_FOLDERS On)
cmake_minimum_required(VERSION 2.8.3) 

project(vectorfield)

message("CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")

# FIND OPENGL
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)


# Set output paths
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#file (COPY ${CMAKE_SOURCE_DIR}/../shaders DESTINATION ${CMAKE_BINARY_DIR} )

include_directories(
	external/glfw-3.0.3/include/GLFW/
	external/glm-0.9.4.0/
	external/glew-1.9.0/include/
	../
	.
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	GLFW_303
	GLEW_190
)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

# Compile external dependencies 
add_subdirectory (external)

# Main app
SET( srcs 
		
		../Program.cpp
		../Material.cpp
		../VectorField.cpp
		../Mesh.cpp
		Glutils.cpp
		Camera.cpp
		main.cpp 
		)

# Headers
SET( headers
		../INIReader.h
		../Program.h
		../Material.h
		../VectorField.h
		../Mesh.h
		Glutils.h
		Camera.h
		)

add_definitions(-DCMAKE_BUILD_TYPE=Debug -DPRINT_DEBUG)
add_definitions(-DSHADER_DIR=\"${CMAKE_SOURCE_DIR}/../shaders/\")

add_executable(vectorfield ${srcs} ${headers})

# create the program
target_link_libraries(vectorfield ${ALL_LIBS} )

source_group("app" FILES Camera.h Camera.cpp GLInlcude.h GLUtils.h GLUtils.cpp main.cpp)