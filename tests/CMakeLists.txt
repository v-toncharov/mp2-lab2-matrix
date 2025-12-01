set(target matrix-tests)
file(GLOB sources CONFIGURE_DEPENDS *.cxx *.cpp *.cc)
add_executable(${target} ${sources})
target_link_libraries(${target} gtest)
