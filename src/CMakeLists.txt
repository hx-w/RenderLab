add_subdirectory(libs)
list(APPEND LIBS NURBS_LIB)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS}) 
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")

file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
install(TARGETS ${PROJECT_NAME} DESTINATION bin) 
install(FILES ${HEADERS} DESTINATION include)