#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="${PROJECT_ROOT:-/root/code}"
if [[ "${PROJECT_ROOT}" == "/root/code" && ! -d "${PROJECT_ROOT}" ]]; then
	PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
fi

cd "${PROJECT_ROOT}"

if [[ -d "/build_libs" ]]; then
	CONFIGURE_PRESET="docker-debug"
	BUILD_PRESET="docker-datapro2"
	EXTRA_CONFIGURE_ARGS=()
else
	CONFIGURE_PRESET="host-debug-full"
	BUILD_PRESET="host-debug-datapro2"
	EXTRA_CONFIGURE_ARGS=()
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
fi

cmake --preset "${CONFIGURE_PRESET}" "${EXTRA_CONFIGURE_ARGS[@]}"
cmake --build --preset "${BUILD_PRESET}" --parallel "$(nproc)"
