"""Host software-in-the-loop regression; executes C, no hardware latency claims."""
from pathlib import Path
import re
import subprocess

root = Path(__file__).resolve().parents[1]
subprocess.run(["make", "all", "test"], cwd=root, check=True)
output = subprocess.check_output([str(root / "bms_firmware_sim")], cwd=root, text=True)
rows = [line for line in output.splitlines() if line.startswith("[t=")]
assert len(rows) == 30
for row in rows:
    ms = int(re.search(r"t=(\d+)ms", row).group(1))
    if 800 <= ms < 1800:
        assert "OFF (LOW)" in row and "State: FAULT" in row, row
    else:
        assert "ON (HIGH)" in row and "State: NORMAL" in row, row
print("PASS: 30 host simulation states checked; no hardware timing measured.")
