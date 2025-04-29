import subprocess
import statistics
import time

# Path to your executable
binary_path = "src/RelWithDebInfo/MyBoostFibersExecutable.exe"

total_time = 0.0
num_runs = 5  # Number of times to run each configuration
times = []
for _ in range(num_runs):
    start_time = time.perf_counter()
    subprocess.run([binary_path, str(10000000), str(16), str(4)], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    end_time = time.perf_counter()
    times.append(end_time - start_time)
    total_time += (end_time - start_time)

average_time = total_time / num_runs
average_time *= 1000  # Convert to milliseconds
print(f"Average Time: {int(average_time)} milliseconds")
# standard deviation
std_dev = statistics.stdev(times)
std_dev *= 1000  # Convert to milliseconds
print(f"Standard Deviation: {int(std_dev)} milliseconds")
