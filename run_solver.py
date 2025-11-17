import subprocess
import csv
import os

def run_solver(file_path, timeout):
    """Runs the solver and returns True if solved (exit code 0), False otherwise."""
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

        # RETURN TRUE IF SUCCESS (solver returns 1 based on your code)
        return result.returncode == 1

    except Exception:
        return False


def main():
    sudoku_types = ["25x25"]  #["9x9", "16x16", "25x25"]

    type_prefix = {
        "9x9": "inst9x9",
        "16x16": "inst16x16",
        "25x25": "inst25x25"
    }

    percentages = range(40, 51, 5) # range(0, 101, 5)
    instances = range(0, 100)

    output_file = "sudoku_results.csv"

    with open(output_file, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["Type", "Clue %", "Total Instances", "Successes", "Success Rate (%)"])

        for t in sudoku_types:
            prefix = type_prefix[t]

            # ------------------------------
            # Set timeout per Sudoku type
            # ------------------------------
            if t == "9x9":
                timeout = "5"
            elif t == "16x16":
                timeout = "20"
            else:
                timeout = "120"
            # ------------------------------

            for pct in percentages:
                total = 0
                success_count = 0

                print(f"Running {t} at {pct}% clues...")

                for inst in instances:
                    file_path = f"instances/general/{prefix}_{pct}_{inst}.txt"

                    solved = run_solver(file_path, timeout)
                    total += 1
                    if solved:
                        success_count += 1
                        print("|", end="", flush=True)
                    else:
                        print("x", end="", flush=True)
                    if inst == 99:
                        print("")

                success_rate = (success_count / total) * 100

                writer.writerow([t, pct, total, success_count, f"{success_rate:.2f}"])

                print(f" → {success_rate:.2f}% success")

    print(f"\nDONE! Results saved to {output_file}")


if __name__ == "__main__":
    main()
