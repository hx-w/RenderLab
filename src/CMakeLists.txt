add_subdirectory(libs)
list(APPEND LIBS NURBS_LIB)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS}) 
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")