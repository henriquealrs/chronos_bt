set(TEST_SOURCES
    order_book_tests.cpp
    csv_parse_tests.cpp
    feeder_test.cpp
    feed_handler_tests.cpp
)

set(POPULATED_TEST_SOURCES "")
foreach(test_source IN LISTS TEST_SOURCES)
    set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/${test_source}")
    if(EXISTS "${source_path}")
        file(READ "${source_path}" source_contents)
        string(LENGTH "${source_contents}" source_length)
        if(source_length GREATER 0)
            list(APPEND POPULATED_TEST_SOURCES "${source_path}")
        endif()
    endif()
endforeach()

if(POPULATED_TEST_SOURCES STREQUAL "")
    message(STATUS "No populated test sources found; skipping test targets.")
    return()
endif()

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(FEED_HANDLER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/feed_handler/feed_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/feed_handler/input_parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/feed_handler/publisher.cpp
)

add_executable(feed_tests ${POPULATED_TEST_SOURCES} ${FEED_HANDLER_SOURCES})
target_link_libraries(feed_tests GTest::gtest_main chronos_utils)
target_include_directories(feed_tests PRIVATE ../core)
add_test(NAME feed_tests COMMAND feed_tests)

