# add dependencies via CPM
include(${PROJECT_SOURCE_DIR}/cmake/CPM.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/OpenMP.cmake)

# GLAD
CPMAddPackage(
  NAME glad
  GITHUB_REPOSITORY Dav1dde/glad
  VERSION 0.1.35
)

# GLM
CPMAddPackage(
  NAME glm
  GITHUB_REPOSITORY g-truc/glm
  GIT_TAG 0.9.9.8
)

# ImGui
CPMAddPackage(
  NAME imgui
  GITHUB_REPOSITORY ocornut/imgui
  VERSION 1.89.1
)

# GLFW
CPMAddPackage(
  NAME glfw
  GITHUB_REPOSITORY glfw/glfw
  GIT_TAG 3.3.8
  OPTIONS
  "GLFW_BUILD_TESTS Off"
  "GLFW_BUILD_EXAMPLES Off"
  "GLFW_BUILD_DOCS Off"
  "GLFW_INSTALL Off"
  "GLFW_USE_HYBRID_HPG On"
)

# get pybind11 and build
CPMAddPackage(
  NAME pybind11
  GITHUB_REPOSITORY pybind/pybind11
  VERSION 2.10.2
)

# put all external targets into a seperate folder to not pollute the project folder
set_target_properties(glad glad-generate-files glfw PROPERTIES FOLDER ExternalTargets)

# Source file grouping of visual studio and xcode
CPMAddPackage(
  NAME GroupSourcesByFolder.cmake
  GITHUB_REPOSITORY TheLartians/GroupSourcesByFolder.cmake
  VERSION 1.0
)

# get imnodes and build
CPMAddPackage(
  NAME imnodes
  GITHUB_REPOSITORY Nelarius/imnodes
  VERSION 0.5
)

CPMAddPackage(
  NAME imguizmo
  GITHUB_REPOSITORY BrutPitt/imGuIZMO.quat
  VERSION 3.0
)

CPMAddPackage(
  NAME tinynurbs
  GITHUB_REPOSITORY hx-w/tinynurbs
  GIT_TAG v0.1.1
)

#-----------------------------------------------------------------------------#
# define the asset path in c++
set(ASSETS_DIR ${PROJECT_SOURCE_DIR}/resource)
add_compile_definitions(ASSETS_PATH="${ASSETS_DIR}")

#-----------------------------------------------------------------------------#
# list of all source files
# set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
# file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "${SRC_DIR}/*.h")
# file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "${SRC_DIR}/*.cpp" "${SRC_DIR}/*.hpp")
# file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS "${ASSETS_DIR}/shader/*.fs" "${ASSETS_DIR}/shader/*.vs")


# ---- configure ImGui ----
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp    # for this task
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp # use glfw & opengl3
)

target_link_libraries(imgui PUBLIC glfw)
target_include_directories(imgui INTERFACE ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_compile_definitions(imgui PUBLIC -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS) # optional imgui setting
set_target_properties(imgui PROPERTIES CXX_STANDARD 17) # use c++17

add_library(imnodes STATIC ${imnodes_SOURCE_DIR}/imnodes.cpp)
target_link_libraries(imnodes PUBLIC imgui glfw)

add_library(imguizmo STATIC ${imguizmo_SOURCE_DIR}/imGuIZMO.quat/imGuIZMOquat.cpp)
target_link_libraries(imguizmo PUBLIC imgui glfw glm)
## !!!important
target_compile_definitions(imguizmo PUBLIC -DIMGUIZMO_IMGUI_FOLDER=)
target_compile_definitions(imguizmo PUBLIC -DVGIZMO_USES_GLM=)

# add_library(tinynurbs STATIC ${tinynurbs_SOURCE_DIR}/tests/test_curve.cpp)
target_link_libraries(tinynurbs INTERFACE glm)
