set(Boost_ADDITIONAL_VERSIONS "1.34" "1.34.0")

find_package(Boost 1.34 REQUIRED COMPONENTS unit_test_framework)


add_executable(testlib testlib.cpp test_outs.hpp)
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
target_compile_features(testlib PRIVATE cxx_std_17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
target_compile_options(testlib
  PUBLIC
    "-O0"
    "-fprofile-arcs"
    "-ftest-coverage"
)
add_test(NAME test_read COMMAND testlib)
add_test(NAME test_write COMMAND testlib)
add_test(NAME test_functions COMMAND testlib)

set(GCOV gcov)
set(LCOV ~/tmp/lcov/bin/lcov)
set(GENHTML ~/tmp/lcov/bin/genhtml)
if (DEFINED ENV{LCOV})
    set(LCOV $ENV{LCOV})
endif()
if (DEFINED ENV{GENHTML})
    set(GENHTML $ENV{GENHTML})
endif()


add_custom_target(testlibrun COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS testlib)
add_custom_target(
    coverage
  COMMAND
    ${GCOV} ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/testlib.dir/testlib.gcno && 
    ${LCOV} --capture --directory ${CMAKE_BINARY_DIR}/.. --output-file ${CMAKE_BINARY_DIR}/LCOV_OUTPUT.info --include */midifile/* &&
    ${GENHTML} ${CMAKE_BINARY_DIR}/LCOV_OUTPUT.info --output-directory ${CMAKE_BINARY_DIR}/CODE_COVERAGE
  DEPENDS testlibrun 
)

add_custom_target(
    save_out
  COMMAND
    ${CMAKE_CURRENT_SOURCE_DIR}/save_out.py ${CMAKE_CURRENT_BINARY_DIR}/out ${CMAKE_CURRENT_SOURCE_DIR}
)

file(
  COPY
    ${CMAKE_CURRENT_SOURCE_DIR}/files
  DESTINATION
    ${CMAKE_CURRENT_BINARY_DIR})

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/out)
