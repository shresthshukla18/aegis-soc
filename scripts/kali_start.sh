#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$ROOT/build/aegis_soc"
if [[ ! -x "$BIN" ]]; then
  echo "AEGIS executable not found. Run ./scripts/kali_setup.sh first." >&2
  exit 1
fi
# Live packet capture needs privileges. Re-exec only when --live is requested.
needs_root=0
for arg in "$@"; do [[ "$arg" == "--live" ]] && needs_root=1; done
if (( needs_root )) && [[ ${EUID:-$(id -u)} -ne 0 ]]; then
  exec sudo -E "$0" "$@"
fi
cd "$ROOT"
exec "$BIN" "$@"
