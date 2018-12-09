
add_executable(day06
    day06.cpp
    ../util/file_parsing.cpp
    )

target_link_libraries(day06
    TBB::tbb
    )
