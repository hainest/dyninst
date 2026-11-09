#! /bin/bash

set -e

function usage {
  echo "Usage: $0 --src-dir [--install-dir] [--build-dir] [-j] [-c] [-t TYPE] [-h] [-v]"
}

function show_help {
  usage
  echo <<EOF
  --src-dir      Source directory
  --install-dir  Installation directory
  --build-dir    Build directory
  -j             Number of CMake build jobs (default: 1)
  -c             CMake arguments (must be quoted: -c "-D1 -D2")
  -t             Build tests (TYPE must be one of ALL, REGRESSION, INTEGRATION, or UNIT)
  -h             Show this help
  -v             Verbose outputs
EOF
}

src_dir=
install_dir=
build_dir=
save_build_dir="no"
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
      save_build_dir="yes"
      shift 2;;
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

if [[ -z "${src_dir}" ]]; then
  echo "No source directory specified"
  show_help
  exit 1
fi
echo "Using source in ${src_dir}"

if [[ -z "${build_dir}" ]]; then
  build_dir=$(mktemp -d "/tmp/XXXXXX")
elif [[ ! -d "${build_dir}" ]]; then
  mkdir -p "${build_dir}"
fi
echo "Building in ${build_dir}"

if [[ ! -z "${install_dir}" ]]; then
  cmake_args+="-DCMAKE_INSTALL_PREFIX=${install_dir} "
  echo "Installing in ${install_dir}"
fi

# Containers run with Docker's default seccomp profile, which does not permit
# unprivileged user namespaces. Before the caller's arguments, so they can override it.
cmake_args+="-DDYNINST_ENABLE_USERNS_TESTS=OFF "

cmake_args+="-DDYNINST_ENABLE_TESTS=${test_type} "
cmake_args+="-DDYNINST_WARNINGS_AS_ERRORS=ON "
cmake_args+="-DDYNINST_ENABLE_FILEFORMAT_PE=ON "
cmake_args+="-DDYNINST_ENABLE_CAPSTONE=ON "

cmake -S "${src_dir}" -B "${build_dir}" ${cmake_args}

cmake --build "${build_dir}" --parallel "${num_jobs}" ${verbose}

if [[ ! -z "${install_dir}" ]]; then
  cmake --install "${build_dir}"
fi

if [[ "${save_build_dir}" = "yes" ]]; then
  # The DSOs are large so remove them from the build directory, but
  # keep the individual object files to reduce future build times.
  find "${build_dir}" -type f -name "*.so" -delete
else
  # assume we're not currently in the build directory'
  rm -rf "${build_dir}"
fi
