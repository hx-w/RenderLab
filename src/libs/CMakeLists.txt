add_subdirectory(infrastructure)
add_subdirectory(geometry)

target_link_libraries(GEOMETRY PRIVATE glm pybind11::embed)
