## 🚀 Live Demo

**[Open AEGIS SOC Dashboard](https://aegis-soc-demo.onrender.com)**

Explore the hosted SOC dashboard in simulation mode.
No installation required.



# AEGIS SOC

**A lightweight C++20 network intrusion-detection prototype with a browser-based SOC dashboard.**

AEGIS SOC is an educational, defensive network-monitoring project. It combines a C++ detection engine, a local HTTP API, a responsive dashboard, synthetic simulation, configurable rules, and optional live packet capture through libpcap.

> **Project status:** prototype / educational use. This is not a production-ready IDS/IPS and does not perform inline packet blocking.

## Dashboard preview

Start the application in simulation mode and open `http://127.0.0.1:8787` in your browser. The dashboard polls the local API for traffic and event updates. Simulation mode uses generated data; it is not evidence of live network detection.

## Features

- C++20 application built with CMake.
- Browser-based dashboard served locally by the C++ application.
- Simulation mode for exploring the dashboard without packet capture privileges.
- Optional libpcap capture for Ethernet II / IPv4 / TCP / UDP traffic.
- Configurable destination-port rules in `config/rules.conf`.
- Basic port-scan and SYN-rate heuristics.
- Local API endpoints: `/api/health`, `/api/status`, `/api/events`, `/api/traffic`, and `/api/rules`.
- Kali Linux / VirtualBox setup scripts, including two independent sensor processes.

## Quick start: Kali Linux

From the extracted project directory:

```bash
chmod +x scripts/*.sh
./scripts/kali_setup.sh
./scripts/kali_start.sh
```

Open **http://127.0.0.1:8787** in Firefox. Stop the server with `Ctrl+C`.

The setup script installs Debian/Kali build dependencies using `apt-get`, then configures and builds the C++ application with CMake. It requires administrator privileges for package installation.

## Live packet capture

Identify interfaces first:

```bash
./scripts/kali_interfaces.sh
sudo tcpdump -D
```

Start one sensor (replace `eth1` with an interface you own or are authorized to monitor):

```bash
sudo ./scripts/kali_start.sh --live eth1 --port 8787
```

### Two independent sensors

For a VirtualBox Kali VM configured with NAT (`eth0`), Host-only (`eth1`) and Bridged (`eth2`):

```bash
sudo ./scripts/kali_start_dual.sh eth1 eth2
```

- Lab sensor: http://127.0.0.1:8787
- LAN sensor: http://127.0.0.1:8788

These are **two independent processes** with separate in-memory event histories. They are not centrally correlated. A bridged VM does not automatically receive all unicast traffic on a switched LAN; whole-LAN visibility generally requires an authorized SPAN/mirror port or network TAP. Wi-Fi bridging and promiscuous mode do not guarantee that visibility.

See [`docs/VIRTUALBOX_KALI_DEPLOYMENT.md`](docs/VIRTUALBOX_KALI_DEPLOYMENT.md) for the full VirtualBox walkthrough.

## API smoke tests

With a sensor running:

```bash
curl http://127.0.0.1:8787/api/health
curl http://127.0.0.1:8787/api/status
curl http://127.0.0.1:8787/api/events
```

A healthy instance returns JSON similar to:

```json
{"ok":true,"service":"Aegis SOC"}
```

## Demo

1. Run `./scripts/kali_start.sh` (simulation mode).
2. Open `http://127.0.0.1:8787`.
3. Observe generated traffic and sample events.
4. In a second terminal, run `curl http://127.0.0.1:8787/api/health` and `curl http://127.0.0.1:8787/api/status`.
5. Stop the application with `Ctrl+C`.

For a live demo, use an isolated lab you control and follow the deployment guide. Do not run tests against networks or devices without explicit authorization.

## Build manually

Requirements: CMake 3.20+, a C++20 compiler (GCC 11+ or Clang 14+), pthreads, and optionally libpcap development headers.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/aegis_soc
```

To build without libpcap, use `-DAEGIS_ENABLE_PCAP=OFF`. Simulation mode remains available.

## Repository layout

```text
include/       C++ interfaces
src/           C++ implementation
web/           dashboard HTML, CSS and JavaScript
config/        detection rules
scripts/       build, setup, startup and diagnostics scripts
docs/          deployment documentation
.github/       GitHub Actions workflow
```

## Limitations and security notes

This prototype is passive. It does **not** block or drop packets and should not be described as an inline IPS. Current limitations include:

- No IPv6 packet inspection, VLAN parsing, TCP stream reassembly, TLS decryption, or comprehensive protocol decoding.
- No persistent database; event history is held in memory and is lost when the process exits.
- No user authentication, role-based access control, or HTTPS.
- No central correlation between the two sensor processes.
- Detection heuristics are basic and can produce false positives or miss attacks. They are not a substitute for a mature engine such as Suricata or Zeek.
- The local dashboard/API should remain bound to loopback and must not be exposed to untrusted networks.
- Packet capture may collect sensitive information. Capture only traffic you own or are explicitly authorized to monitor, and handle packet data according to applicable law and policy.

## License

Released under the [MIT License](LICENSE).
