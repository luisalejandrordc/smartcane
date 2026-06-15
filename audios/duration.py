#!/usr/bin/env python3

import math
import re
import subprocess
from pathlib import Path

duration_pattern = re.compile(r"estimated duration:\s*([\d.]+)\s*sec")

for i in range(1, 22):
    filename = f"{i:04d}.mp3"
    path = Path(filename)

    if not path.exists():
        print(f"{filename}: NOT FOUND")
        continue

    try:
        result = subprocess.run(
            ["afinfo", str(path)], capture_output=True, text=True, check=True
        )

        match = duration_pattern.search(result.stdout)
        if match:
            duration = float(match.group(1))
            new_duration = (
                math.ceil(duration * 10) / 10 + 0.500
            )  # half a second of margin
            print(f"{filename}: {duration:.3f} sec -> {new_duration:.3f} sec")
        else:
            print(f"{filename}: Duration not found")

    except subprocess.CalledProcessError as e:
        print(f"{filename}: Error running afinfo ({e})")
