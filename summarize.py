import pandas as pd
import os

# List of input CSV filenames to process
input_files = [
    "ablation-t0_1.5.csv", "ablation-t0_1.csv",  "ablation-t0_2.csv",
]

# Dimensions you expect in each dataset
dimensions = ["6x6", "9x9", "12x12", "16x16", "25x25"]

# Ensure summary directory exists
os.makedirs("summary", exist_ok=True)

for file in input_files:
    print(f"Processing {file} ...")

    # Load dataset
    df = pd.read_csv(file)

    summary_rows = []

    for dim in dimensions:
        subset = df[df["Type"] == dim]
        if subset.empty:
            continue

        # Simple means (success rate, time, std dev, cycles)
        success_rate = subset["Success Rate (%)"].mean()
        avg_time = subset["Avg Time"].mean()
        avg_sd_time = subset["Std Dev Time"].mean()
        avg_cycles = subset["Cycles"].mean()

        summary_rows.append({
            "Dimension": dim,
            "Success Rate (%)": round(success_rate, 2),
            "Avg Time (s)": avg_time,
            "Std Dev Time (s)": avg_sd_time,
            "Cycles": round(avg_cycles, 2)
        })

    # Convert and save summary
    summary_df = pd.DataFrame(summary_rows)
    output_file = os.path.join("summary", f"SUMMARY-{os.path.basename(file)}")
    summary_df.to_csv(output_file, index=False)

    print(f"Saved: {output_file}")

print("All summaries completed successfully!")
