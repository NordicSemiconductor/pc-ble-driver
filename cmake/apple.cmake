set(Boost_USE_STATIC_LIBS ON)

add_compile_options(-pthread -std=c++11)

set(CMAKE_SKIP_BUILD_RPATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "@loader_path")

