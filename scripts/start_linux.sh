#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
if [[ ! -x build/aegis_soc ]]; then echo "Executable missing. Run ./scripts/build_linux.sh first."; exit 1; fi
exec ./build/aegis_soc "$@"
