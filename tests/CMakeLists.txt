set(Boost_ADDITIONAL_VERSIONS "1.34" "1.34.0")

find_package(Boost 1.34 REQUIRED COMPONENTS unit_test_framework)

add_executable(testlib testlib.cpp)
target_include_directories(
  testlib
  PRIVATE
    ${Boost_INCLUDE_DIRS}
)
target_compile_definitions(
  testlib
  PRIVATE
    "BOOST_TEST_DYN_LINK=1"
)
target_link_libraries(
  testlib
  PRIVATE
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    gcov
    midifile
)
target_compile_options(testlib
  PUBLIC
    "-O0"
    "-fprofile-arcs"
    "-ftest-coverage"
)
add_test(NAME test1 COMMAND testlib)

# add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS testlib)

file(
  COPY
    ${CMAKE_CURRENT_SOURCE_DIR}/files
  DESTINATION
    ${CMAKE_CURRENT_BINARY_DIR})
