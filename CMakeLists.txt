cmake_minimum_required(VERSION 3.0.0)
project(sexy-raytracer VERSION 0.1.0)

include_directories(./eigen)
include_directories(./cgltf)
include_directories(./stb)
include_directories(./glad/build/include)
include_directories(./glfw/include)

link_directories(./glad/build)
link_directories(./glfw/build/src)

include(CTest)
enable_testing()

add_executable(sexy-raytracer main.cpp globals.cpp)

target_link_libraries(sexy-raytracer glfw3 glad dl pthread GL)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
