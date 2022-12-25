add_subdirectory(infrastructure)
add_subdirectory(tooth)
add_subdirectory(render)
list(APPEND LIBS INFRASTRUCTURE XTOOTH XRENDER)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
include(${PROJECT_SOURCE_DIR}/cmake/dependency.cmake)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

GroupSourcesByFolder(${PROJECT_NAME})


# link deps to libs
target_link_libraries(XRENDER PRIVATE glad glfw glm imgui INFRASTRUCTURE)
target_link_libraries(XTOOTH PRIVATE glm INFRASTRUCTURE)

target_link_libraries(${PROJECT_NAME} PRIVATE glad glfw glm imgui ${LIBS})
include_directories(${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)

# include

target_include_directories(XTOOTH PRIVATE ${INFRASTRUCTURE_INCLUDE_DIRS})
target_include_directories(XRENDER PRIVATE ${INFRASTRUCTURE_INCLUDE_DIRS})