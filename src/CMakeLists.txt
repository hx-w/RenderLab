add_subdirectory(libs)
add_subdirectory(infrastructure)
add_subdirectory(tooth)
add_subdirectory(render)
list(APPEND LIBS NURBS_LIB INFRASTRUCTURE TOOTH_ENGINE XRENDER)

set_target_properties(${TARGET} PROPERTIES COMPILE_FLAGS "-pthread" LINK_FLAGS "-pthread")

add_executable(${PROJECT_NAME} main.cpp execute.hpp)
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS}) 
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")
