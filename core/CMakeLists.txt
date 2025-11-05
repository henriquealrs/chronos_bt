file (GLOB FEED_HNDL_SRC "feed_handler/*.cpp")

set(MATCHING_SOURCES
    matching_engine.cpp
)

find_package(PkgConfig REQUIRED)
find_package(RapidJSON  CONFIG REQUIRED)
pkg_check_modules(ZMQ REQUIRED libzmq)

set(_rapidjson_target "")
set(_rapidjson_include_dirs "")

add_library(chronos_utils STATIC
    utils/thread_utils.cpp
    utils/zmq_utils.cpp
)
target_include_directories(chronos_utils
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${ZMQ_INCLUDE_DIRS}
        cppzmq
)
target_link_libraries(chronos_utils
    PUBLIC
        ${ZMQ_LIBRARIES}
)

function(configure_chronos_executable target_name)
    target_link_libraries(${target_name} PRIVATE ${ZMQ_LIBRARIES})
    target_include_directories(${target_name} PRIVATE ${ZMQ_INCLUDE_DIRS} cppzmq)
    target_compile_options(${target_name}
        PRIVATE
            $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -O0 -g >
            $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    )
    # Prefer imported target(s) if provided by the RapidJSON config
    if(TARGET RapidJSON)
        target_link_libraries(${target_name} PRIVATE RapidJSON)
    elseif(TARGET rapidjson)
        target_link_libraries(${target_name} PRIVATE rapidjson)
    elseif(DEFINED RapidJSON_INCLUDE_DIRS)
        target_include_directories(${target_name} PRIVATE ${RapidJSON_INCLUDE_DIRS})
    elseif(DEFINED RAPIDJSON_INCLUDE_DIRS)
        target_include_directories(${target_name} PRIVATE ${RAPIDJSON_INCLUDE_DIRS})
    else()
        message(FATAL_ERROR
            "RapidJSON was found but exposed no usable targets/vars. "
            "Check your RapidJSON installation.")
    endif()
endfunction()

add_executable(feed_handler ${FEED_HNDL_SRC})
target_link_libraries(feed_handler PRIVATE chronos_utils)
configure_chronos_executable(feed_handler)

add_executable(matching ${MATCHING_SOURCES})
target_link_libraries(matching PRIVATE chronos_utils)
configure_chronos_executable(matching)
