# Building and running CRISP on macOS

CRISP is a 32-bit Windows program. On macOS it is compiled with `clang-cl` and `lld-link`
against the Visual Studio 2008 headers and libraries, and run with Wine.

`build.py` reads the source lists from the VS2008 `.vcproj` files and mirrors the VS2008
Release flags, so both builds compile the same code. The VS2008 solution still works as before.

## Setup

1. `brew install llvm@22 lld@22`
2. Copy these folders from a Visual Studio 2008 install into `mac/vs2008/`:

   | From (on Windows)                                              | To                          |
   |----------------------------------------------------------------|-----------------------------|
   | `Microsoft Visual Studio 9.0\VC\include`                       | `vc/include`                |
   | `Microsoft Visual Studio 9.0\VC\lib` (32-bit files only)       | `vc/lib`                    |
   | `Microsoft Visual Studio 9.0\VC\atlmfc\include`                | `vc/atlmfc/include`         |
   | `Microsoft Visual Studio 9.0\VC\atlmfc\lib` (32-bit only)      | `vc/atlmfc/lib`             |
   | `Microsoft SDKs\Windows\v6.0A\Include`                         | `sdk/include`               |
   | `Microsoft SDKs\Windows\v6.0A\Lib` (32-bit only)               | `sdk/lib`                   |

3. `mac/setup.sh` builds the JWasm assembler, downloads Wine into `mac/tools/wine`, and
   patches MFC's `.rc` files for `llvm-rc`.

## Build and run

    mac/run.sh                  # build Release and start CRISP
    mac/build.py                # build only; outputs in mac/build/Release/bin
    mac/build.py --config Debug
    mac/build.py -v CRISP2      # print the commands for one project

CRISP runs from `mac/build/<config>/run`, together with the support files in `runtime/`.

## Differences from the VS2008 build

- Precompiled headers are emulated with a VFS overlay, because clang resolves
  `#include "stdafx.h"` by path while MSVC matches it by name.
- `CRISP2.Src/Complex/BM_templ.cpp` and `CRISP2.Src/nfft.cpp` generate inline assembly from
  macros. They are preprocessed and rejoined into whole `_asm` blocks before compiling, at `/O1`.
- MFC `TOOLBAR` resources are converted to the equivalent `RT_TOOLBAR` binary data for `llvm-rc`.
