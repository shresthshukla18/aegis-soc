# AEGIS SOC demo

This is a reproducible demonstration of the **simulation mode**, not a recording of real network traffic.

## Run the demo

From the repository root:

```bash
./scripts/demo.sh
```

The script starts AEGIS in the background, waits for the local health endpoint, prints a status and events response, and then stops the process. To inspect the dashboard interactively instead, run:

```bash
./scripts/kali_start.sh
```

Open `http://127.0.0.1:8787` in Firefox and allow a few seconds for simulated telemetry to populate. Stop with `Ctrl+C`.

## Record a short GitHub demo video/GIF

1. Start the interactive simulation command above.
2. Capture the browser showing the dashboard and changing simulated telemetry using your desktop screen recorder.
3. Keep the recording to 20–40 seconds; do not show real IP addresses, credentials, or private traffic.
4. Save the result as `demo/aegis-dashboard-demo.gif` (or MP4) and add it to the repository.
5. Update the README Demo section to embed it, for example: `![AEGIS SOC demo](demo/aegis-dashboard-demo.gif)`.

Do not label simulation output as real attack detection or real packet capture.
