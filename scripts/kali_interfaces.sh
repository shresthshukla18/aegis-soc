#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
echo "=== Linux interfaces and addresses ==="
ip -br addr || true
echo
echo "=== Routing table ==="
ip route || true
echo
echo "=== libpcap capture devices ==="
if command -v tcpdump >/dev/null 2>&1; then
  if [[ ${EUID:-$(id -u)} -eq 0 ]]; then tcpdump -D || true; else sudo tcpdump -D || true; fi
else
  echo "tcpdump not installed; run ./scripts/kali_setup.sh"
fi
echo
echo "Use the interface/device names shown above. Names such as enp0s3/enp0s8/enp0s9 are examples only."
