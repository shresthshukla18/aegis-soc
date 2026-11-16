# AEGIS SOC — Kali Linux on VirtualBox deployment guide

This guide deploys the supplied C++20 AEGIS prototype in a Kali VM with 4 GB RAM, a browser, and three VirtualBox network adapters. It supports simulation, a host-only lab capture process, and a separate bridged-interface capture process.

## 0. Important scope and safety

- This is an educational/prototype passive IDS. It does **not** implement inline packet blocking or a production IPS.
- Each live process captures one interface. `kali_start_dual.sh` starts two independent processes and two independent in-memory dashboards; it is **not** a central correlated dashboard.
- The web server binds to `127.0.0.1` only. Keep it local; do not change this to `0.0.0.0` or expose it to a network. There is no login/RBAC or TLS.
- Only monitor networks and devices you own or are explicitly authorized to inspect. Captures can contain sensitive data.
- VirtualBox promiscuous mode is not a network tap. Bridged capture sees traffic delivered to Kali (including Kali's own traffic and any traffic explicitly mirrored to it); it does not magically reveal all unicast traffic between other LAN clients. For whole-LAN visibility use a managed switch SPAN/mirror port or an authorized network TAP, with a suitable wired interface. Wi-Fi client mode commonly cannot expose arbitrary other clients' unicast frames.

## 1. VirtualBox VM resources

Power off the Kali VM completely (not Saved State), then configure:

- RAM: 4096 MB. If the host has 8 GB total, avoid running all VMs at once.
- CPUs: 2 virtual CPUs (do not allocate more than the host can spare).
- Disk: 30 GB dynamically allocated recommended; keep adequate host free space.
- Display: enable 3D acceleration only if stable; not required for IDS.

### Adapter 1 — NAT (management/internet)

VirtualBox → Kali VM → Settings → Network → Adapter 1:

- Enable Network Adapter: checked
- Attached to: NAT
- Cable Connected: checked
- Adapter Type: Intel PRO/1000 MT Desktop (82540EM) is a reasonable default

Use this for apt updates and ordinary internet access. It is not a feed of all LAN traffic.

### Adapter 2 — Host-only (isolated lab)

Create a host-only network in VirtualBox Manager → Tools → Network → Host-only Networks (menu wording varies by VirtualBox release). Create one if none exists. Give it a private subnet, for example `192.168.56.0/24`, and enable the VirtualBox DHCP server if you want automatic addressing. Avoid overlapping with your real LAN or VPN routes.

Kali VM → Settings → Network → Adapter 2:

- Enable Network Adapter: checked
- Attached to: Host-only Adapter
- Name: the host-only network you created (often `vboxnet0`)
- Promiscuous Mode: Allow VMs (or Allow All if the VirtualBox version only offers that and you understand the broader exposure)
- Cable Connected: checked

Attach each lab target/test VM to the **same** host-only network. For those VMs, use only this adapter for the isolated exercise unless they separately need controlled updates. Do not bridge the test VMs to your physical LAN.

### Adapter 3 — Bridged (authorized physical LAN)

Kali VM → Settings → Network → Adapter 3:

- Enable Network Adapter: checked
- Attached to: Bridged Adapter
- Name: select the host's actual wired Ethernet adapter where possible
- Promiscuous Mode: Allow All (if available)
- Cable Connected: checked

Bridged mode makes Kali another participant on that LAN. It does not guarantee visibility into unrelated devices' traffic. For whole-LAN monitoring, connect the host's wired NIC to a switch SPAN/mirror destination port or a network TAP and ensure the host/VirtualBox bridge can pass the mirrored frames. Do not assume a Wi-Fi bridge can provide monitor-mode capture.

### Routing note

With multiple DHCP-enabled adapters, Linux may install multiple default routes. Keep the NAT adapter as the preferred default route for internet access. The host-only adapter should have a connected private subnet route and normally no default gateway. The bridged adapter may receive a LAN default gateway; if it does, adjust its NetworkManager IPv4 route metric or DHCP route setting so NAT remains preferred. Do not blindly delete routes; inspect `ip route` first. Packet capture itself does not require the capture interface to be the default route.

## 2. Install and build

Extract the ZIP in Kali, then:

```bash
cd ~/Downloads/cpp_ids_soc
chmod +x scripts/*.sh
./scripts/kali_setup.sh
```

The setup script uses apt to install `build-essential`, `cmake`, `pkg-config`, `libpcap-dev`, `tcpdump`, `curl`, `iproute2`, and CA certificates, then configures and builds the C++20 executable with PCAP support when detected.

To rebuild after changing code:

```bash
./scripts/build_linux.sh
```

## 3. Discover actual interface names — do not assume names

```bash
./scripts/kali_interfaces.sh
# or individually:
ip -br link
ip -br addr
ip route
sudo tcpdump -D
```

Match the VirtualBox adapter MAC addresses to the Linux interface MAC addresses shown by `ip link`. Typical names such as `enp0s3`, `enp0s8`, and `enp0s9` are examples only. In the common order, Adapter 1 is NAT, Adapter 2 host-only, Adapter 3 bridged, but verify this on your VM.

## 4. First launch: simulation

```bash
./scripts/kali_start.sh
```

Open Firefox inside Kali at `http://127.0.0.1:8787`. Simulation mode creates synthetic events and is suitable for checking the UI. Stop with Ctrl+C.

## 5. Live capture: isolated lab only

Identify the host-only interface from Step 3, then:

```bash
sudo ./scripts/kali_start.sh --live <HOST_ONLY_INTERFACE> --port 8787
```

Example only (replace the interface):

```bash
sudo ./scripts/kali_start.sh --live enp0s8 --port 8787
```

Open `http://127.0.0.1:8787` in Firefox. Generate benign test traffic between your own lab VMs. The current parser handles Ethernet II, IPv4, TCP and UDP only; it is not a complete protocol dissector. It does not currently provide IPv6, VLAN parsing, TCP stream reassembly, TLS inspection, or inline blocking.

## 6. Two independent live sensors: host-only + bridged

Use two separate terminal windows only if you want to inspect the lab and the traffic visible on the bridged adapter simultaneously. Run as root because libpcap capture typically needs elevated privileges:

```bash
sudo ./scripts/kali_start_dual.sh <HOST_ONLY_INTERFACE> <BRIDGED_INTERFACE>
```

Example only:

```bash
sudo ./scripts/kali_start_dual.sh enp0s8 enp0s9
```

Dashboards:

- Lab sensor: `http://127.0.0.1:8787`
- Bridged sensor: `http://127.0.0.1:8788`

The script writes process output to `logs/lab-sensor.log` and `logs/lan-sensor.log`. Press Ctrl+C in that terminal to stop both. The dashboards are independent and events are currently held in memory, so they are lost on restart. Do not run two processes on the same port.

## 7. Verify

With each sensor running, open a second terminal:

```bash
curl -sS http://127.0.0.1:8787/api/health
curl -sS http://127.0.0.1:8788/api/health
curl -sS http://127.0.0.1:8787/api/status
curl -sS http://127.0.0.1:8788/api/status
```

A health response should contain `"ok":true`. A missing response on port 8788 is expected if only the single sensor on 8787 is running.

## 8. Test in the isolated lab

1. Start Kali sensor on its host-only interface.
2. Start one target VM on the same host-only network.
3. From a second authorized test VM on that same host-only network, run a small number of connection checks against the target's IP, e.g. `nc -vz <TARGET_IP> 22` or `ping <TARGET_IP>`.
4. Inspect dashboard events and the status API. A connection attempt is not itself proof of an intrusion. The current port-scan heuristic requires repeated distinct destination ports from a source; thresholds are prototype logic.

Do not run tests against third-party systems or production networks without explicit authorization.

## 9. Troubleshooting

- `PCAP support is not compiled in`: run `sudo apt install libpcap-dev`, then `./scripts/build_linux.sh`.
- `Could not start packet capture`: confirm interface spelling with `sudo tcpdump -D`; run live mode with sudo; check the interface is UP.
- Health endpoint unavailable: inspect the terminal/logs, check port conflicts with `ss -ltnp | grep -E '8787|8788'`, and ensure process is still running.
- No packets on host-only: ensure both target and test VMs use the same host-only network and cable is connected. Check traffic on Kali with `sudo tcpdump -ni <interface>`.
- No unrelated LAN traffic on bridged: expected without a SPAN/TAP feed. Promiscuous mode alone cannot make a switch forward other clients' unicast packets to Kali.
- Wrong internet route: inspect `ip route`; prefer NAT for default route and do not configure a default gateway on the host-only network.

## 10. Current project limitations

- Two-interface mode is two independent application instances, not a central event aggregator.
- No persistent database, authentication/RBAC, TLS, alert delivery, or audit log yet.
- No actual packet dropping/blocking. The project is passive IDS, not an inline IPS.
- Capture parser currently supports Ethernet II + IPv4 + TCP/UDP only; it is not a production-grade packet parser.
- HTTP API is intended for localhost use only. Keep it bound to loopback.
- Prototype alerts are heuristics and require validation; absence of alerts does not prove a network is safe.
