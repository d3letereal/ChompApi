# CMakeList.txt : CMake project for ChompFramework, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (ChompFramework "ChompFramework.cpp" "ChompFramework.h" "window/Window.h" "window/Window.cpp" "objects/Cube.cpp" "objects/Cube.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ChompFramework PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
