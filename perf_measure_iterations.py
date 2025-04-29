import subprocess
import time

# Path to your executable
binary_path = "src/Release/MyBoostFibersExecutable.exe"

# Initial and final iteration counts
start_iterations = 10000
end_iterations = 10000000
factor = 2  # Multiplier for each step
num_runs = 5  # Number of times to run each configuration
fixed_arg = 100  # Fixed second argument

for network_num in range(6, 8):
    print(f"Network number: {network_num}")
    iterations = start_iterations
    while iterations <= end_iterations:
        total_time = 0.0
        for _ in range(num_runs):
            start_time = time.perf_counter()
            subprocess.run([binary_path, str(iterations), str(fixed_arg), str(network_num)], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            end_time = time.perf_counter()
            total_time += (end_time - start_time)

        average_time = total_time / num_runs
        print(f"Iterations: {iterations}, Average Time: {average_time:.6f} seconds")

        iterations *= factor
    print("\n")
