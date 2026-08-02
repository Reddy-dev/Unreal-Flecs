#!/usr/bin/env python3
"""Entry point for compiler-aware Flecs test conversion.

The implementation lives beside this launcher in the Unreal-Flecs plugin's
Scripts directory so the conversion tool and its policy stay scoped to the
plugin. Generated C++ is written below Source/FlecsLibrary/Tests/Generated.
"""

from flecs_test_converter import main


if __name__ == "__main__":
    raise SystemExit(main())
