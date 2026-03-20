# RotorHazard Integration

Guide for connecting FPVGate to a [RotorHazard](https://github.com/RotorHazard/RotorHazard) race timer server.

**Navigation:** [Home](../README.md) | [User Guide](USER_GUIDE.md) | [Hardware Guide](HARDWARE_GUIDE.md) | [Features](FEATURES.md)

---

## Overview

FPVGate can operate as an external RSSI node for RotorHazard. When enabled, the two systems stay in sync:

- **FPVGate to RH** - Gate crossings are submitted as laps; race start/stop commands are forwarded
- **RH to FPVGate** - RotorHazard race lifecycle events (stage, start, stop) are relayed back to control FPVGate's timer
- **Clock Sync** - An NTP-style clock synchronization keeps timing aligned between both systems

This allows you to use FPVGate's portable RSSI hardware while recording results in RotorHazard's full race management system.

---

## Requirements

- FPVGate firmware **v1.7.0** or later
- RotorHazard **4.x** (tested on 4.4.0+)
- FPVGate and RotorHazard on the **same local network** (FPVGate in WiFi Station mode)
- The [FPVGate RH Plugin](https://github.com/LouisHitchcock/fpvgate-rh-plugin) installed on the RH server

---

## Setup

### Step 1: Install the RH Plugin

The companion plugin runs on the RotorHazard server and handles lap recording and race control relay.

**Full plugin documentation and source:** [github.com/LouisHitchcock/fpvgate-rh-plugin](https://github.com/LouisHitchcock/fpvgate-rh-plugin)

Quick install:

1. Clone or download the plugin into the RotorHazard plugins directory:

   ```bash
   cd /home/pi/RotorHazard/src/server/plugins
   git clone https://github.com/LouisHitchcock/fpvgate-rh-plugin.git fpvgate
   ```

2. Restart the RotorHazard server:

   ```bash
   sudo systemctl restart rotorhazard
   ```

3. Check the server log for confirmation:

   ```
   FPVGate plugin v1.0.0 initialized
   ```

### Step 2: Connect FPVGate to Your Network

FPVGate must be on the same network as the RotorHazard server. In the FPVGate web UI:

1. Go to **Settings > WiFi & Connection**
2. Set **Connection Mode** to **Station** (join existing network)
3. Enter your WiFi SSID and password
4. Click **Apply** (device will reboot)
5. Reconnect to FPVGate on its new IP address

### Step 3: Enable RotorHazard Mode

1. Go to **Settings > WiFi & Connection > Sync**
2. Under **This Device**, change the role dropdown to **RotorHazard**
3. The RotorHazard configuration panel will appear with these fields:

   **RH Host IP** - The IP address of your RotorHazard server (e.g. `192.168.1.50`). Port 5000 is used automatically.

   **Node Seat (0-7)** - Which RotorHazard seat/node this FPVGate corresponds to. Must match the pilot slot in your RH heat setup.

4. Click **Save Configuration**

### Step 4: Verify Connection

1. Click **Refresh** next to the Connection Status badge
   - **Connected** (green) - FPVGate can reach the RH server
   - **Disconnected** (red) - Check the RH Host IP and network connectivity

2. Click **Sync Now** to perform a manual clock synchronization
   - **Synced (RTT: Xms)** - Clock sync successful. RTT under 10ms is ideal on a local network.
   - **Sync failed** - Check that the RH plugin is installed and the server is running

---

## How It Works

### Race Flow

1. **Start a race from FPVGate** - FPVGate sends a stage command to RH with a synchronized `startTimeMs` timestamp. RH uses this to align its race clock with FPVGate's, bypassing the countdown delay.

2. **Gate crossing detected** - FPVGate queues the crossing with the raw `millis()` timestamp. On the next loop tick, it POSTs the lap to `POST /fpvgate/lap` on the RH server with the node index and timing data.

3. **Stop a race from FPVGate** - FPVGate sends a stop command to RH.

4. **Start/stop from RotorHazard** - The RH plugin detects race lifecycle events and sends HTTP commands back to FPVGate (`/timer/start`, `/timer/stop`, `/timer/clearLaps`, `/timer/countdown`).

### Clock Synchronization

FPVGate performs an NTP-style clock sync with the RH server:

1. FPVGate records local time `t1`, sends `GET /fpvgate/time` to RH
2. RH responds with `{"monotonic_ms": <server_time>}`
3. FPVGate records local time `t2` when the response arrives
4. Round-trip time: `RTT = t2 - t1`
5. Clock offset: `offset = server_time - (t1 + RTT/2)`
6. Any local `millis()` value can be converted to RH time: `rh_time = millis() + offset`

Sync happens:
- Immediately when WiFi connects
- Every 30 seconds during idle (no pending laps)
- On manual trigger from the web UI

### Lap Payload

```json
{
  "node": 0,
  "raceTimeMs": 12345,
  "timestampMs": 987654321
}
```

- `node` - Zero-based seat index matching the RH heat assignment
- `raceTimeMs` - Milliseconds since race start (fallback timing)
- `timestampMs` - Absolute RH monotonic timestamp (only when clock is synced)

### Race Control Endpoints

| Direction | Endpoint | Purpose |
|---|---|---|
| FPVGate to RH | `POST /fpvgate/lap` | Submit gate crossing |
| FPVGate to RH | `POST /fpvgate/race/stage` | Stage/arm a new race |
| FPVGate to RH | `POST /fpvgate/race/stop` | Stop the current race |
| RH to FPVGate | `POST /timer/start` | Start the lap timer |
| RH to FPVGate | `POST /timer/stop` | Stop the lap timer |
| RH to FPVGate | `POST /timer/clearLaps` | Clear lap list |
| RH to FPVGate | `POST /timer/countdown` | Play pre-race countdown |
| FPVGate to RH | `GET /fpvgate/time` | Clock sync request |

---

## Troubleshooting

**Connection status shows "Disconnected"**
- Verify the RH Host IP is correct
- Ensure FPVGate is in Station mode on the same network
- Check that the RH server is running and accessible on port 5000
- Verify the FPVGate RH plugin is installed: check RH logs for `FPVGate plugin v1.0.0 initialized`

**Clock sync fails**
- The plugin must be installed for the `/fpvgate/time` endpoint to exist
- Check that the RH server is not overloaded (high RTT indicates network issues)
- Try a manual sync from the web UI

**Laps not appearing in RotorHazard**
- Laps are only recorded when RH is in `RACING` state
- Verify the Node Seat matches the correct pilot slot in the RH heat
- Check the RH server log for `FPVGate lap` entries

**Race start/stop not syncing**
- Both directions require network connectivity
- FPVGate to RH: check the RH Host IP configuration
- RH to FPVGate: the plugin auto-detects FPVGate's IP from the first lap POST. Make sure at least one lap has been sent for reverse communication to work.

**High timing jitter**
- Check the RTT value in the Clock Sync status. Under 10ms is ideal.
- Use a wired Ethernet connection for the RH server if possible
- Avoid congested WiFi channels

---

## Links

- **FPVGate RH Plugin:** [github.com/LouisHitchcock/fpvgate-rh-plugin](https://github.com/LouisHitchcock/fpvgate-rh-plugin)
- **RotorHazard:** [github.com/RotorHazard/RotorHazard](https://github.com/RotorHazard/RotorHazard)
- **FPVGate:** [github.com/LouisHitchcock/FPVGate](https://github.com/LouisHitchcock/FPVGate)
