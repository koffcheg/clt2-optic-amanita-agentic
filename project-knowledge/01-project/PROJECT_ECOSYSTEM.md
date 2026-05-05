# PROJECT_ECOSYSTEM

## Purpose

This file records the project ecosystem: languages, standards, libraries, toolchain, target OS, build/runtime infrastructure, and authoritative source files.

It is the single place for environment knowledge so agents and developers do not need to reconstruct these facts from the whole repository every time.

## Target Platform

The repository and build infrastructure primarily target Linux.

Evidence:
- `builder/Dockerfile` uses `ubuntu:22.04`.
- `docker-compose.yml` uses a containerized Linux workflow.
- `README.md` contains Linux commands, `apt` packages, and POSIX queue paths such as `/dev/mqueue` and `/dev/shm`.

Reproducible build base:
- Ubuntu 22.04 in the Docker build container.

## Languages And Standards

Main code:
- C++ is the primary project language.
- C++20 is configured at the repository root for most modules.
- C++17 is configured separately for `manager`.
- C is present as a secondary language in `manager`.

Auxiliary languages:
- Python is used in `overseer/`.
- Bash / shell scripting is used in `builder/`, `overseer/`, and `install/`.
- CMake is the main build configuration system.
- JSON/XML are used for configuration and parts of runtime input.

Important constraint:
- The project mixes C++ standards: root modules generally use C++20, while `manager` uses C++17. Account for this when changing shared code, interfaces, or compile assumptions.

## Main Libraries And Dependencies

Computer vision / image processing:
- OpenCV 4.9.0.
- `opencv_world` is used.
- Dockerfile enables `xfeatures2d` and `OPENCV_ENABLE_NONFREE`.

Logging:
- Apache log4cxx 1.3.1.

Utility / formatting / geometry:
- Boost 1.85.0.
- fmt 11.0.2.
- Clipper2 1.4.0.

Data/config parsing:
- Jansson.

Runtime/system:
- `pthread`.
- POSIX IPC primitives are referenced in `README.md` through `/dev/mqueue` and `/dev/shm`.

Testing:
- GoogleTest is the target C++ unit-test framework.
- CTest is the target CMake-level test runner when tests are integrated into the build.

## Build System

CMake is the main build system.

Known facts:
- root `cmake_minimum_required(VERSION 3.16)`;
- `manager` locally requires `3.18`.

Containerized build uses:
- `builder/Dockerfile`;
- `docker-compose.yml`;
- `builder/build_all.sh`;
- `builder/build_datapro1.sh`;
- `builder/build_datapro2.sh`;
- other `builder/*.sh`.

The local manual build path is documented in the root `README.md`.

## AI-Agent Build Playbook

The goal of this section is a reproducible build process that is not tied to one user or machine.

### Command Approval Rule

Before every command that changes system state or filesystem state, ask the user for explicit approval.

This includes:
- installing or updating system packages (`apt-get install`, `apt-get upgrade`, ...);
- building and installing libraries from source;
- `cmake` configure, including reruns with different parameters;
- `cmake --build` or any other build command;
- filesystem writes outside the repository.

Read-only commands may be run without asking. Examples: search, version checks, `dpkg -l`, `find`, `cat`, `cmake --debug-find-pkg`.

Do not run several state-changing steps under one approval. One approval equals one step.

### Step 1. Check Dependencies, Read-Only

System packages:

```bash
dpkg -l | grep -E 'libeigen3|libopenexr|libjansson'
```

Custom-prefix CMake package search:

```bash
find / -name "OpenCVConfig.cmake" 2>/dev/null
find / -name "log4cxxConfig.cmake" 2>/dev/null
find / -name "Clipper2Config.cmake" 2>/dev/null
find / -name "fmtConfig.cmake" 2>/dev/null
find / -name "boost_filesystem-config.cmake" -o -name "BoostConfig.cmake" 2>/dev/null
```

Record found prefixes or mark dependencies as missing.

### Step 2. Check Library Versions

Minimum project requirements:

| Library | Minimum version |
|---|---|
| Boost | 1.85.0 |
| OpenCV | 4.9.0, static `opencv_world` recommended |
| log4cxx | 1.3.1 |
| Clipper2 | TODO: confirm with user; check `ClipperVersion.h` |
| fmt | 11.0.2 approximately |
| jansson | >= 2.x system package |

Version check pattern:

```bash
grep -r "version" <FOUND_PREFIX>/lib/cmake/<Pkg>/<Pkg>ConfigVersion.cmake 2>/dev/null
```

### Step 3. Install Missing System Packages, With Approval

Only after user approval:

```bash
sudo apt-get update
sudo apt-get install -y libeigen3-dev libopenexr-dev libjansson-dev
```

### Step 4. Build Missing Custom-Prefix Libraries, With Approval

If Boost, OpenCV, log4cxx, Clipper2, or fmt are missing:
1. Read the root `README.md`; it contains the canonical build steps.
2. Prepare and show commands for approval.
3. Execute one step at a time with approval before each step.

Do not run multiple steps without intermediate approval.

### Step 5. Configure, With Approval

Canonical configure flow uses `CMakePresets.json`.

Examples:

```bash
cmake --preset dp1dp2-debug
cmake --preset dp1dp2-release
```

`builder/*.sh` wrapper scripts are allowed as a compatibility layer if they delegate to preset flow.

Before each configure command, show it to the user and wait for approval.

If static OpenCV `opencv_world` requires transitive CMake targets `Eigen3::Eigen` or `OpenEXR::OpenEXR` and they are not defined, use a non-invasive workaround before configure:

```cmake
# /tmp/opencv_deps.cmake
find_package(Eigen3 CONFIG REQUIRED)
find_package(OpenEXR CONFIG REQUIRED)
```

General configure template:

```bash
cmake -S <SRC_DIR> -B <BUILD_DIR>   -DBoost_NO_SYSTEM_PATHS=ON   -DBOOST_ROOT=<BOOST_PREFIX>   -Dlog4cxx_DIR=<LOG4CXX_PREFIX>/lib/cmake/log4cxx   -DOpenCV_DIR=<OPENCV_PREFIX>/lib/cmake/opencv4   -DClipper2_DIR=<CLIPPER2_PREFIX>/lib/cmake/clipper2   [-DCMAKE_PROJECT_INCLUDE=<opencv_deps.cmake>]
```

### Step 6. Build, With Approval

Canonical build flow:

```bash
cmake --build --preset dp1dp2-debug --parallel "$(nproc)"
cmake --build --preset dp1dp2-release --parallel "$(nproc)"
```

Docker/CI-like environment:

```bash
cmake --preset docker-debug
cmake --build --preset docker-all --parallel "$(nproc)"
```

Show the command and wait for user approval before running it.

## Authoritative Source Files

Environment facts should be checked against:
- `README.md`;
- root `CMakeLists.txt`;
- `CMakePresets.json`;
- `builder/Dockerfile`;
- `docker-compose.yml`;
- `builder/*.sh`;
- relevant module `CMakeLists.txt`.
