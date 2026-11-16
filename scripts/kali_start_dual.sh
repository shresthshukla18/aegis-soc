#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$ROOT/build/aegis_soc"
if [[ ! -x "$BIN" ]]; then echo "Run ./scripts/kali_setup.sh first." >&2; exit 1; fi
if [[ $# -lt 2 || $# -gt 4 ]]; then
  echo "Usage: sudo ./scripts/kali_start_dual.sh <host-only-interface> <bridged-interface> [lab-port] [lan-port]" >&2
  echo "Example: sudo ./scripts/kali_start_dual.sh enp0s8 enp0s9 8787 8788" >&2
  exit 2
fi
if [[ ${EUID:-$(id -u)} -ne 0 ]]; then exec sudo -E "$0" "$@"; fi
LAB_IF="$1"; LAN_IF="$2"; LAB_PORT="${3:-8787}"; LAN_PORT="${4:-8788}"
if [[ "$LAB_IF" == "$LAN_IF" ]]; then echo "Choose two different interfaces." >&2; exit 2; fi
for iface in "$LAB_IF" "$LAN_IF"; do
  if ! ip link show dev "$iface" >/dev/null 2>&1; then echo "Interface not found: $iface" >&2; exit 2; fi
done
if [[ "$LAB_PORT" == "$LAN_PORT" ]]; then echo "Dashboard ports must differ." >&2; exit 2; fi
mkdir -p "$ROOT/logs"
PIDS=()
cleanup(){
  echo "Stopping AEGIS sensor processes..."
  for pid in "${PIDS[@]:-}"; do kill -TERM "$pid" 2>/dev/null || true; done
  for pid in "${PIDS[@]:-}"; do wait "$pid" 2>/dev/null || true; done
}
trap cleanup EXIT INT TERM
cd "$ROOT"
"$BIN" --live "$LAB_IF" --port "$LAB_PORT" >"$ROOT/logs/lab-sensor.log" 2>&1 & PIDS+=("$!")
"$BIN" --live "$LAN_IF" --port "$LAN_PORT" >"$ROOT/logs/lan-sensor.log" 2>&1 & PIDS+=("$!")
sleep 2
for port in "$LAB_PORT" "$LAN_PORT"; do
  if ! curl --silent --fail "http://127.0.0.1:$port/api/health" >/dev/null; then
    echo "Sensor on port $port did not become healthy. Check logs/ and interface permissions." >&2
    exit 1
  fi
done
echo "AEGIS dual-sensor mode is running."
echo "Lab dashboard: http://127.0.0.1:$LAB_PORT (interface $LAB_IF)"
echo "LAN dashboard: http://127.0.0.1:$LAN_PORT (interface $LAN_IF)"
echo "Logs: logs/lab-sensor.log and logs/lan-sensor.log"
echo "Press Ctrl+C to stop both sensors."
while kill -0 "${PIDS[0]}" 2>/dev/null && kill -0 "${PIDS[1]}" 2>/dev/null; do sleep 1; done
echo "A sensor process exited. Check its log."
