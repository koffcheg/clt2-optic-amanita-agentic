#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

BUILD_TYPE="${1:-Debug}"
case "${BUILD_TYPE}" in
  Debug|Release) ;;
  *)
    echo "Usage: $0 [Debug|Release]"
    exit 1
    ;;
esac

BUILD_DIR="${PROJECT_ROOT}/build-dp1dp2-${BUILD_TYPE,,}"
DEPS_PRELOAD_FILE="${BUILD_DIR}/opencv_deps_preload.cmake"

mkdir -p "${BUILD_DIR}"
cat > "${DEPS_PRELOAD_FILE}" <<'EOF'
find_package(Eigen3 CONFIG REQUIRED)
find_package(OpenEXR CONFIG REQUIRED)
EOF

CMAKE_ARGS=(
  -S "${PROJECT_ROOT}"
  -B "${BUILD_DIR}"
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
  -DCMAKE_PROJECT_INCLUDE="${DEPS_PRELOAD_FILE}"
)

if [[ -n "${LOG4CXX_DIR:-}" ]]; then
  CMAKE_ARGS+=("-Dlog4cxx_DIR=${LOG4CXX_DIR}")
fi

if [[ -n "${OPENCV_DIR:-}" ]]; then
  CMAKE_ARGS+=("-DOpenCV_DIR=${OPENCV_DIR}")
fi

if [[ -n "${BOOST_ROOT:-}" ]]; then
  CMAKE_ARGS+=(
    "-DBoost_NO_SYSTEM_PATHS=ON"
    "-DBOOST_INCLUDEDIR=${BOOST_ROOT}/include"
    "-DBOOST_LIBRARYDIR=${BOOST_ROOT}/lib"
    "-DBOOST_ROOT=${BOOST_ROOT}"
    "-DBoost_ROOT=${BOOST_ROOT}"
    "-DCMAKE_POLICY_DEFAULT_CMP0144=NEW"
  )
fi

cmake "${CMAKE_ARGS[@]}"
cmake --build "${BUILD_DIR}" --target datapro1 datapro2 -j"$(nproc)"

echo "Build completed: ${BUILD_DIR} (${BUILD_TYPE})"
