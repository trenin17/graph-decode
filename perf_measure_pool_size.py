import subprocess
import time

# Path to your executable
binary_path = "build_release/src/Release/MyBoostFibersExecutable.exe"

# Initial and final iteration counts
start_pool_size = 3
end_pool_size = 2000
factor = 2  # Multiplier for each step
num_runs = 5  # Number of times to run each configuration
fixed_arg = 1000000  # Fixed first argument

pool_size = start_pool_size
while pool_size <= end_pool_size:
    total_time = 0.0
    for _ in range(num_runs):
        start_time = time.perf_counter()
        subprocess.run([binary_path, str(fixed_arg), str(pool_size), str(4)], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        end_time = time.perf_counter()
        total_time += (end_time - start_time)

    average_time = total_time / num_runs
    print(f"Pool size: {pool_size}, Average Time: {average_time:.6f} seconds")

    pool_size *= factor
