#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
if ! command -v apt-get >/dev/null 2>&1; then
  echo "This installer expects Kali/Debian with apt-get." >&2
  exit 1
fi
if [[ ${EUID:-$(id -u)} -ne 0 ]]; then
  echo "Installing system packages requires sudo."
  exec sudo -E bash "$0" "$@"
fi
export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get install -y --no-install-recommends \
  build-essential cmake pkg-config libpcap-dev tcpdump curl iproute2 net-tools ca-certificates
chmod +x "$ROOT"/scripts/*.sh
cmake -S "$ROOT" -B "$ROOT/build" -DCMAKE_BUILD_TYPE=Release -DAEGIS_ENABLE_PCAP=ON
cmake --build "$ROOT/build" --parallel "$(nproc 2>/dev/null || echo 2)"
echo
echo "[OK] AEGIS SOC compiled."
echo "Simulation: $ROOT/scripts/kali_start.sh"
echo "Live:       sudo $ROOT/scripts/kali_start.sh --live <interface> --port 8787"
echo "Devices:    sudo $ROOT/scripts/kali_interfaces.sh"
