cmake_minimum_required(VERSION 3.0)
project(Glitter)


if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++11 -Wno-dangling-reference -Wno-implicit-fallthrough")
    if(NOT WIN32)
        set(GLAD_LIBRARIES dl)
    endif()
endif()

option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(Glitter/Vendor/glfw)

#option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
#option(ASSIMP_BUILD_SAMPLES OFF)
#option(ASSIMP_BUILD_TESTS OFF)
#add_subdirectory(Glitter/Vendor/assimp)
option(BUILD_BULLET2_DEMOS OFF)
option(BUILD_CPU_DEMOS OFF)
option(BUILD_EXTRAS OFF)
option(BUILD_OPENGL3_DEMOS OFF)
option(BUILD_UNIT_TESTS OFF)
add_subdirectory(Glitter/Vendor/bullet)

include_directories(Glitter/Headers/
                    Glitter/Vendor/bullet/src/
                    Glitter/Vendor/glad/include/
                    Glitter/Vendor/glfw/include/
                    Glitter/Vendor/glm/
                    Glitter/Vendor/stb/
					Glitter/Vendor/imgui/
					Glitter/Vendor/imgui/backends/)


file(GLOB VENDORS_SOURCES Glitter/Vendor/glad/src/glad.c)
file(GLOB PROJECT_HEADERS Glitter/Headers/*.hpp
                          Glitter/Headers/*.h)

file(GLOB PROJECT_SOURCES Glitter/Sources/*.cpp
                          Glitter/Sources/*.hpp
						  Glitter/Vendor/imgui/*.cpp
						  Glitter/Vendor/imgui/backends/*.cpp
						  Glitter/Vendor/imgui/*.hpp
						)
file(GLOB PROJECT_SHADERS Glitter/Shaders/*.comp
                          Glitter/Shaders/*.frag
                          Glitter/Shaders/*.geom
                          Glitter/Shaders/*.vert)
file(GLOB PROJECT_CONFIGS CMakeLists.txt
                          Readme.md
                         .gitattributes
                         .gitignore
                         .gitmodules)

source_group("Headers" FILES ${PROJECT_HEADERS})
source_group("Shaders" FILES ${PROJECT_SHADERS})
source_group("Sources" FILES ${PROJECT_SOURCES})
source_group("Vendors" FILES ${VENDORS_SOURCES})

add_definitions(-DGLFW_INCLUDE_NONE
                -DPROJECT_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"
                -Wno-dangling-reference)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS}
                               ${PROJECT_SHADERS} ${PROJECT_CONFIGS}
                               ${VENDORS_SOURCES})
find_library(ASSIMP_LIBRARY assimp  ${PROJECT_SOURCE_DIR}/lib/)
target_link_libraries(${PROJECT_NAME} ${ASSIMP_LIBRARY} glfw
                      ${GLFW_LIBRARIES} ${GLAD_LIBRARIES} ${OPENGL_LIBRARIES}
                      BulletDynamics BulletCollision LinearMath)
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME})

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Glitter/Shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>
    DEPENDS ${PROJECT_SHADERS})

# 设定库和可执行文件的路径
set(LIBRARY_PATH "${CMAKE_SOURCE_DIR}/lib/libassimp-5.dll")
set(EXECUTABLE_PATH "${CMAKE_SOURCE_DIR}/build/Glitter")
set(LIBRARY_PATH_STDCPP "E:/mingw64/bin/libstdc++-6.dll")
set(LIBRARY_PATH_GCC "E:/mingw64/bin/libgcc_s_seh-1.dll")
set(LIBRARY_PATH_WINPTHREAD "E:/mingw64/bin/libwinpthread-1.dll")

# 在可执行文件构建之后复制动态库
add_custom_command(
	TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${LIBRARY_PATH} $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${LIBRARY_PATH_STDCPP} $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${LIBRARY_PATH_GCC} $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${LIBRARY_PATH_WINPTHREAD} $<TARGET_FILE_DIR:${PROJECT_NAME}>
)