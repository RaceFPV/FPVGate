"""
Add PlatformIO's mklittlefs tool directory to PATH so buildfs/uploadfs find
the binary when it isn't on system PATH (e.g. pioarduino platform).
Must be Python: extra_scripts run in SCons and need env; bash cannot modify the build env.
"""
import os
from pathlib import Path

Import("env")

def find_mklittlefs_dir():
    for base in (
        Path(env.get("PROJECT_DIR", "")) / ".platformio" / "packages",
        Path.home() / ".platformio" / "packages",
    ):
        if not base.is_dir():
            continue
        for d in base.iterdir():
            if not d.is_dir() or "tool-mklittlefs" not in d.name or "mklittlefs4" in d.name:
                continue
            if (d / "mklittlefs").is_file() or (d / "mklittlefs.exe").is_file():
                return str(d)
        for d in base.iterdir():
            if not d.is_dir() or "tool-mklittlefs" not in d.name:
                continue
            for name in ("mklittlefs", "mklittlefs.exe"):
                if (d / name).is_file():
                    return str(d)
    return None

tool_dir = find_mklittlefs_dir()
if tool_dir:
    env.PrependENVPath("PATH", tool_dir)
    print("[set_mklittlefs_path] Added to PATH: %s" % tool_dir)
else:
    print("[set_mklittlefs_path] WARNING: mklittlefs not found in packages; buildfs may fail.")
