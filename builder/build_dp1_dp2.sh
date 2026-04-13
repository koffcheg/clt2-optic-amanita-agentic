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

if [[ "${BUILD_TYPE}" == "Debug" ]]; then
  CONFIGURE_PRESET="dp1dp2-debug"
  BUILD_PRESET="dp1dp2-debug"
else
  CONFIGURE_PRESET="dp1dp2-release"
  BUILD_PRESET="dp1dp2-release"
fi

declare -a EXTRA_CONFIGURE_ARGS=()

if [[ -n "${LOG4CXX_DIR:-}" ]]; then
  EXTRA_CONFIGURE_ARGS+=("-Dlog4cxx_DIR=${LOG4CXX_DIR}")
fi

if [[ -n "${OPENCV_DIR:-}" ]]; then
  EXTRA_CONFIGURE_ARGS+=("-DOpenCV_DIR=${OPENCV_DIR}")
fi

if [[ -n "${BOOST_ROOT:-}" ]]; then
  EXTRA_CONFIGURE_ARGS+=(
    "-DBoost_NO_SYSTEM_PATHS=ON"
    "-DBOOST_INCLUDEDIR=${BOOST_ROOT}/include"
    "-DBOOST_LIBRARYDIR=${BOOST_ROOT}/lib"
    "-DBOOST_ROOT=${BOOST_ROOT}"
    "-DBoost_ROOT=${BOOST_ROOT}"
    "-DCMAKE_POLICY_DEFAULT_CMP0144=NEW"
  )
fi

cmake --preset "${CONFIGURE_PRESET}" "${EXTRA_CONFIGURE_ARGS[@]}"
cmake --build --preset "${BUILD_PRESET}" --parallel "$(nproc)"

echo "Build completed via presets: ${BUILD_PRESET} (${BUILD_TYPE})"
