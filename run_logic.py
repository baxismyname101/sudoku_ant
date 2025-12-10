import subprocess
import csv
import os
import statistics

FOLDER = "instances/logic-solvable"
OUTPUT_FILE = "logic_solvable_results.csv"
RUNS_PER_FILE = 100

def run_solver(file_path, timeout):
    """Runs solver and returns solved?, time, cycles."""
    cmd = [
        "./sudokusolver",
        "--file", file_path,
        "--timeout", timeout
    ]

    try:
        result = subprocess.run(
            cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
            text=True
        )

        out = result.stdout.strip().split("\n")
        solved = (result.returncode == 0)

        time_value = float(out[1]) if solved and len(out) > 1 else None
        cycles = int(out[2]) if solved and len(out) > 2 else None

        return solved, time_value, cycles

    except Exception:
        return False, None, None


def main():
    # Auto-detect puzzle files in the folder
    puzzle_files = sorted(f for f in os.listdir(FOLDER) if f.endswith(".txt"))

    with open(OUTPUT_FILE, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow([
            "File", "Type", "Total Runs", "Successes",
            "Success Rate (%)", "Avg Time", "Std Dev Time", "Avg Cycles"
        ])

        for filename in puzzle_files:
            file_path = os.path.join(FOLDER, filename)
            print(f"\nRunning: {filename}")

            times = []
            cycles_list = []
            success_count = 0

            # Infer type from filename (if possible)
            if "6" == filename[0]:
                puzzle_type = "6x6"
            elif "1" == filename[0]:
                puzzle_type = "12x12"
            else:
                puzzle_type = "9x9"

            # Smart timeouts per type
            timeout = "5" if puzzle_type == "9x9" else \
                      "10" if puzzle_type == "12x12" else \
                      "20" if puzzle_type == "16x16" else \
                      "120" if puzzle_type == "25x25" else "3"

            for i in range(RUNS_PER_FILE):

                solved, t_value, cycles = run_solver(file_path, timeout)
                if solved:
                    success_count += 1
                    if t_value is not None: times.append(t_value)
                    if cycles is not None: cycles_list.append(cycles)

                print("|" if solved else "x", end="", flush=True)

            # Stats
            success_rate = (success_count / RUNS_PER_FILE) * 100
            avg_time = statistics.mean(times) if times else 0
            std_time = statistics.stdev(times) if len(times) > 1 else 0
            avg_cycles = statistics.mean(cycles_list) if cycles_list else 0

            print(f"\n → {success_rate:.2f}%")
            print(f"Avg Time: {avg_time:.8f}")
            print(f"Std Time: {std_time:.8f}")
            print(f"Avg Cycles: {avg_cycles:.2f}")

            # Write CSV row
            writer.writerow([
                filename, puzzle_type, RUNS_PER_FILE, success_count,
                f"{success_rate:.2f}",
                f"{avg_time:.8f}",
                f"{std_time:.8f}",
                f"{avg_cycles:.2f}"
            ])

    print(f"\nDONE! Results saved to {OUTPUT_FILE}")


if __name__ == "__main__":
    main()
