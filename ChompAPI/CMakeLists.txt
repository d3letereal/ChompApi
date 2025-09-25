# CMakeList.txt : CMake project for ChompFramework, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (ChompAPI "ChompFramework.cpp" "ChompFramework.h" "window/Window.h" "window/Window.cpp" "objects/Cube.cpp" "objects/Cube.h" "objects/Skybox.h" "objects/Skybox.cpp" "objects/OBJLoader.h" "objects/Types.h" "objects/Shape.h" "objects/Pyramid.h" "objects/Pyramid.cpp" "customization/Colors.h" "objects/Renderer.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ChompAPI PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
