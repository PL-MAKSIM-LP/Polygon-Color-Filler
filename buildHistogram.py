import matplotlib.pyplot as plt

names = []
times = []

with open("result.txt", "r") as f:
    lines = [line.strip() for line in f if line.strip()]

cpuInfo = lines[0] 
for i in range(1, len(lines), 2):
    name = lines[i]
    value = lines[i+1].replace("ms", "")
    names.append(name)
    times.append(float(value))

plt.figure(figsize=(8, 5))

colors = []
for value in times:
    if value > 800:
        colors.append("orangered")
    elif value > 400:
        colors.append("orange")
    else:
        colors.append("green")

bars = plt.bar(names, times, width=0.2, color=colors)

for bar, time in zip(bars, times):
    plt.text(
        bar.get_x() + bar.get_width() / 2,
        bar.get_height(),
        f"{time:.1f}ms",
        ha='center',
        va='bottom',
        fontsize=10
    )

plt.xlabel(cpuInfo)
plt.ylabel("Milliseconds (ms)")
plt.title("Average execution time")

plt.xticks(rotation=45)
plt.tight_layout()

plt.savefig("execution_time.png", dpi=300)
plt.close()
