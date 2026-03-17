# FPVGate RotorHazard Plugin

Companion plugin for [FPVGate](https://github.com/your-repo/FPVGate) that allows an FPVGate infrared lap timer to submit laps directly into RotorHazard via Socket.IO.

## How It Works

The integration is fully bidirectional:

**FPVGate -> RH (lap recording)**
When the IR gate triggers, the FPVGate firmware fires an HTTP POST to `POST /fpvgate/lap` on the RH server. The plugin records it as a lap via `rhapi.race.lap_add()`. FPVGate's IP is auto-detected from this request — no manual configuration needed.

**RH -> FPVGate (race control)**
The plugin listens to RH race lifecycle events and sends the corresponding command back to FPVGate:

| RH Event | FPVGate Endpoint | Effect |
|---|---|---|
| Race staged | `/timer/clearLaps` + `/timer/countdown` | Clears laps, plays pre-race beeps |
| Race started | `/timer/start` | Starts the lap timer |
| Race stopped / finished | `/timer/stop` | Stops the lap timer |
| Laps manually cleared | `/timer/clearLaps` | Clears lap list |

## Requirements

- RotorHazard 4.x (tested on 4.4.0+)
- FPVGate firmware with RH Integration enabled

## Installation

1. Copy the `fpvgate/` directory into your RotorHazard `plugins/` folder:

   ```
   /home/pi/RotorHazard/src/server/plugins/fpvgate/
   ```

2. Restart the RotorHazard server:

   ```bash
   sudo systemctl restart rotorhazard
   ```

3. Confirm the plugin loaded by checking the server log for:

   ```
   FPVGate plugin v1.0.0 initialized
   ```

## Configuration (FPVGate Side)

In the FPVGate web UI (WiFi & Connection settings):

1. Enable **RH Integration**
2. Set **RH Host IP** to your Raspberry Pi's IP address
3. Set **RH Seat** to the seat number assigned to this gate (0-based)
4. Save and reboot

## Event Payload

The firmware emits a JSON payload over Socket.IO:

```json
{ "node": 0 }
```

- `node` - Zero-based seat/node index matching the RotorHazard heat assignment

## Notes

- Laps are only recorded when a race is in `RACING` state. Gate triggers before or after a race are silently ignored.
- The timestamp is calculated on the server side (time of event receipt minus race start time). Network latency is typically <5ms on a local network, which is well within acceptable timing accuracy for FPV racing.
