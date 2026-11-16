# Security Policy

## Scope

AEGIS SOC is an educational network-monitoring prototype, not a hardened security product. Do not deploy it as the sole security control for a production network.

## Reporting a vulnerability

Please do not publish sensitive exploit details in a public issue. Until a private security contact is configured for this repository, report only non-sensitive reproducible details through a GitHub issue and ask to coordinate disclosure before sharing exploit code or packet captures.

## Safe operation

- Keep the dashboard bound to `127.0.0.1` / loopback.
- Do not expose the HTTP API to the public internet.
- Capture only networks and devices you own or are explicitly authorized to monitor.
- Treat packet captures and logs as potentially sensitive data.
- Run live capture with the minimum privileges required and stop it when testing is complete.
