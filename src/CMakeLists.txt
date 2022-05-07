add_subdirectory(libs)
add_subdirectory(tooth)
add_subdirectory(render)
list(APPEND LIBS NURBS_LIB TOOTH_ENGINE XRENDER)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS}) 
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")