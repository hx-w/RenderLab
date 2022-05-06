add_subdirectory(libs)
add_subdirectory(tooth)
list(APPEND LIBS NURBS_LIB TOOTH_ENGINE)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS}) 
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")