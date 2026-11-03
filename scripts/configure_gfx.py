"""Limit Arduino_GFX to the Qualia RGB implementation used by this project.

Arduino_GFX ships every supported bus and display backend as compilable source.
PlatformIO otherwise compiles all of them, which substantially increases build
time and can pull unrelated platform-specific code into an ESP32-S3 build.
"""

import json
from pathlib import Path

Import("env")

project_dir = Path(env.subst("$PROJECT_DIR"))
pio_env = env.subst("$PIOENV")
gfx_dir = project_dir / ".pio" / "libdeps" / pio_env / "GFX Library for Arduino"

if gfx_dir.exists():
    manifest = {
        "name": "GFX Library for Arduino",
        "version": "1.5.6",
        "build": {
            "srcFilter": [
                "-<*>",
                "+<Arduino_DataBus.cpp>",
                "+<Arduino_G.cpp>",
                "+<Arduino_GFX.cpp>",
                "+<canvas/Arduino_Canvas.cpp>",
                "+<databus/Arduino_XCA9554SWSPI.cpp>",
                "+<databus/Arduino_ESP32RGBPanel.cpp>",
                "+<display/Arduino_RGB_Display.cpp>",
            ]
        },
    }
    manifest_path = gfx_dir / "library.json"
    content = json.dumps(manifest, indent=2) + "\n"
    if not manifest_path.exists() or manifest_path.read_text(encoding="utf-8") != content:
        manifest_path.write_text(content, encoding="utf-8")