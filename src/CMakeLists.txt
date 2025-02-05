find_package(TBB REQUIRED)

# Define the executable
add_executable(MyBoostFibersExecutable main.cpp)

# Link Boost Fibers
target_link_libraries(MyBoostFibersExecutable
    PRIVATE
    TBB::tbb
)
