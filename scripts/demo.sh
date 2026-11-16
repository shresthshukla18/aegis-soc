#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$ROOT/build/aegis_soc"
PORT="${1:-8787}"
if [[ ! -x "$BIN" ]]; then
  echo "Build first: ./scripts/kali_setup.sh" >&2
  exit 1
fi
if ! command -v curl >/dev/null 2>&1; then
  echo "curl is required. Install it with: sudo apt-get install curl" >&2
  exit 1
fi
cd "$ROOT"
echo "Starting AEGIS SOC in simulation mode on http://127.0.0.1:${PORT}"
"$BIN" --port "$PORT" &
PID=$!
cleanup() { kill -TERM "$PID" 2>/dev/null || true; wait "$PID" 2>/dev/null || true; }
trap cleanup EXIT INT TERM
for _ in {1..20}; do
  if curl --silent --fail "http://127.0.0.1:${PORT}/api/health"; then
    echo
    echo "Dashboard: http://127.0.0.1:${PORT}"
    echo "Status:    http://127.0.0.1:${PORT}/api/status"
    echo "Press Ctrl+C to stop the demo."
    wait "$PID"
    exit 0
  fi
  sleep 0.5
done
echo "The demo server did not become healthy. Check its output above." >&2
exit 1
