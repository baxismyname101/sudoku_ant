import subprocess
import csv
import os
import copy
import statistics

def run_solver(file_path, timeout):
    """Runs the solver and returns (solved, time_value)."""
    cmd = [
        "./sudokusolver",
        "--file", file_path,
        "--timeout", timeout
    ]

    try:
        result = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

        # Extract 2nd line as float (solver prints 0\nTIME)
        out = result.stdout.strip().split("\n")
        time_value  = None
        cycles = None
        if (result.returncode == 0):

            time_value = float(out[1]) if len(out) > 1 else None
            cycles = int(out[2]) if len(out) > 2 else None

        # Print time for each run

        return result.returncode == 0, time_value, cycles

    except Exception:
        return False, None


def main():
    sudoku_types = ["6x6", "9x9", "12x12", "16x16", "25x25"]

    type_prefix = {
        "6x6": "inst6x6",
        "9x9": "inst9x9",
        "12x12": "inst12x12",
        "16x16": "inst16x16",
        "25x25": "inst25x25"
    }

    percentages = range(0, 100, 5)  # range(0, 101, 5)
    instances = range(0, 100)

    output_file = "sudoku_results.csv"

    with open(output_file, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow([
            "Type", "Clue %", "Total Instances", "Successes",
            "Success Rate (%)", "Avg Time", "Std Dev Time", "Cycles"
        ])

        for t in sudoku_types:
            prefix = type_prefix[t]

            # Select timeout based on type
            if t == "9x9":
                timeout = "5"
            elif t == "16x16":
                timeout = "20"
            elif t == "12x12":
                timeout = "10"
            elif t == "6x6":
                timeout = "3"
            else:
                timeout = "120"

            for pct in percentages:
                total = 0
                success_count = 0
                times = []  # store every run time
                iterarr = []

                print(f"Running {t} at {pct}% clues...")

                for inst in instances:
                    file_path = f"instances/general/{prefix}_{pct}_{inst}.txt"

                    solved, t_value, iter = run_solver(file_path, timeout)
                    total += 1

                    if solved:
                        success_count += 1
                        print("|", end="", flush=True)
                    else:
                        print("x", end="", flush=True)

                    if t_value is not None:
                        times.append(t_value)

                    if iter is not None:
                        iterarr.append(iter)

                    if inst == 99:
                        print("")

                success_rate = (success_count / total) * 100

                # Calculate average and standard deviation
                avg_time = statistics.mean(times) if times else 0
                std_time = statistics.stdev(times) if len(times) > 1 else 0
                avg_iter = statistics.mean(iterarr) if iterarr else 0

                print(f" → {success_rate:.2f}% success")
                print(f"Average time: {avg_time:.8f}")
                print(f"Std Dev time: {std_time:.8f}")
                print(f"Average Cycles: {avg_iter:.2f}")

                # Write to CSV
                writer.writerow([
                    t, pct, total, success_count,
                    f"{success_rate:.2f}",
                    f"{avg_time:.8f}",
                    f"{std_time:.8f}",
                    f"{avg_iter:.2f}"
                ])

    print(f"\nDONE! Results saved to {output_file}")


if __name__ == "__main__":
    main()
