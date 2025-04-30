include_directories(C:/dev/poc/fibers/libs/boost)
link_directories(C:/dev/poc/fibers/libs/boost/stage/lib)

# Define the executable
add_executable(MyBoostFibersExecutable main.cpp)

# Link Boost Fibers
target_link_libraries(MyBoostFibersExecutable
    PRIVATE
    boost_thread-vc143-mt-x64-1_83.lib
)
