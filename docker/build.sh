#! /bin/bash

set -e

function usage {
  echo "Usage: $0 --src-dir --build-dir [--install-dir] [-j] [-c] [-t TYPE] [-h] [-v]"
}

function show_help {
  usage
  echo <<EOF
  --src-dir         Source directory
  --install-dir     Installation directory (specify multiple with colon-delimited list)
  --build-dir       Build directory
  --fresh-build     Clean the build directory before building
  --clean-artifacts Remove large build artifacts after build
  -j                Number of CMake build jobs (default: 1)
  -c                CMake arguments (must be quoted: -c "-D1 -D2")
  -t                Build tests (TYPE must be one of ALL, REGRESSION, INTEGRATION, or UNIT)
  -h                Show this help
  -v                Verbose outputs
EOF
}

src_dir=
install_dir=
build_dir=
fresh_build="no"
clean_artifacts="no"
num_jobs=$((command nproc 2>/dev/null) || echo "1")
cmake_args=
test_type="OFF"
verbose=

while [[ $# -gt 0 ]]; do
  case "$1" in
    --src-dir)
      src_dir="$2"
      shift 2;;
    --install-dir) 
      install_dir="$2"
      shift 2;;
    --build-dir)
      build_dir="$2"
      shift 2;;
    --fresh-build)
      fresh_build="yes"
      shift;;
    --clean-artifacts)
      clean_artifacts="yes"
      shift;;
    -j) 
      num_jobs="$2"
      shift 2;;
    -c)
      cmake_args="$2"
      shift 2;;
    -t)
      test_type="$2"
      shift 2;;
    -h)
      show_help
      exit;;
    -v)
      verbose="--verbose"
      shift;;
     *)
      echo "Unknown arg '$1'"
      show_help
      exit 1;;
  esac
done

if [[ ! -z "${verbose}" ]]; then
  set -x
fi

for opt in build_dir src_dir; do
  if [[ -z "${!opt}" ]]; then
    echo -e "'${opt}' is required\n"
    show_help
    exit 1
  fi
done
echo "Using source in ${src_dir}"

if [[ "${fresh_build}" = "yes" ]]; then
  rm -rf "${build_dir}"
fi
echo "Building in ${build_dir}"

# Containers run with Docker's default seccomp profile, which does not permit
# unprivileged user namespaces. Before the caller's arguments, so they can override it.
cmake_args+="-DDYNINST_ENABLE_USERNS_TESTS=OFF "

cmake_args+="-DDYNINST_ENABLE_TESTS=${test_type} "
cmake_args+="-DDYNINST_WARNINGS_AS_ERRORS=ON "
cmake_args+="-DDYNINST_ENABLE_FILEFORMAT_PE=ON "
cmake_args+="-DDYNINST_ENABLE_CAPSTONE=ON "

function do_config() {
  args="$1"
  cmake -S "${src_dir}" -B "${build_dir}" ${cmake_args} ${args} 
}
function do_build() {
  cmake --build "${build_dir}" --parallel "${num_jobs}" ${verbose}
}

do_config
do_build

if [[ ! -z "${install_dir}" ]]; then
  IFS=':' read -a dirs <<< "${install_dir}"
  for d in "${dirs[@]}"; do
    echo "Installing in ${d}"
    do_config "-DCMAKE_INSTALL_PREFIX=${d} "
    cmake --install "${build_dir}"
  done
fi

if [[ "${clean_artifacts}" = "yes" ]]; then
  # The DSOs are large so remove them from the build directory, but
  # keep the individual object files to reduce future build times.
  find "${build_dir}" -type f -name "*.so*" -delete
fi
