#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$ROOT/build/aegis_soc"
[[ -x "$BIN" ]] || { echo "Missing build/aegis_soc; run setup."; exit 1; }
echo "=== Build ==="
"$BIN" --help
echo
echo "=== Health endpoint (start AEGIS in another terminal first) ==="
for port in 8787 8788; do
  if curl --max-time 2 --silent --fail "http://127.0.0.1:$port/api/health"; then
    echo "  port $port: OK"
  else
    echo "  port $port: not running (this is not a build failure)"
  fi
done
