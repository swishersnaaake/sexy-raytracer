cmake_minimum_required(VERSION 3.0.0)
project(sexy-raytracer VERSION 0.1.0)

include_directories(/home/lupyenyoung/Documents/work/git/eigen)
include_directories(/home/lupyenyoung/Documents/work/git/cgltf)
include_directories(/home/lupyenyoung/Documents/work/git/stb)
include_directories(/home/lupyenyoung/Documents/work/git/glfw/include)
include_directories(/home/lupyenyoung/Documents/work/git/glad/build/include)

link_directories(/home/lupyenyoung/Documents/work/git/glfw/build/src)
link_directories(/home/lupyenyoung/Documents/work/git/glad/build)

include(CTest)
enable_testing()

add_executable(sexy-raytracer main.cpp gl.cpp)

target_link_libraries(sexy-raytracer glfw3 glad dl pthread)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
