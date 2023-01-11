add_subdirectory(libs)
add_subdirectory(tooth)
add_subdirectory(render)
add_subdirectory(GUI)

list(APPEND LIBS INFRASTRUCTURE XTOOTH XRENDER XGUI GEOMETRY)

list(APPEND DEPS glm imgui glad glfw)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

# install deps
include(${PROJECT_SOURCE_DIR}/cmake/dependency.cmake)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

GroupSourcesByFolder(${PROJECT_NAME})

# link deps to libs
target_link_libraries(XRENDER PRIVATE ${DEPS} INFRASTRUCTURE GEOMETRY)
target_link_libraries(XTOOTH PRIVATE glm INFRASTRUCTURE GEOMETRY)
target_link_libraries(XGUI PRIVATE ${DEPS} INFRASTRUCTURE GEOMETRY XRENDER)

target_link_libraries(${PROJECT_NAME} PRIVATE ${DEPS} ${LIBS} pybind11::embed)
include_directories(${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)

# include
target_include_directories(XTOOTH PRIVATE ${INFRASTRUCTURE_INCLUDE_DIRS} ${GEOMETRY_INCLUDE_DIRS})
target_include_directories(XRENDER PRIVATE ${INFRASTRUCTURE_INCLUDE_DIRS} ${GEOMETRY_INCLUDE_DIRS})
target_include_directories(XGUI PRIVATE ${INFRASTRUCTURE_INCLUDE_DIRS} ${GEOMETRY_INCLUDE_DIRS})