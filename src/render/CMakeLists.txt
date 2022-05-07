# 添加目标链接
if (APPLE)
    set(GLFW_LINK ${CMAKE_CURRENT_SOURCE_DIR}/libs/GLFW/libglfw.3.dylib)
endif()
if (WIN32)
    set(GLFW_LINK ${CMAKE_CURRENT_SOURCE_DIR}/libs/GLFW/glfw3.lib)
endif()

link_libraries(${GLFW_LINK})


# 链接系统的 OpenGL 框架
if (APPLE)
    target_link_libraries(xrender "-framework OpenGL")
endif()

file(GLOB_RECURSE CPPSRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE CSRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
add_library(XRENDER ${CSRCS} ${CPPSRCS})
target_include_directories(XRENDER INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
