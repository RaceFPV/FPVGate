# DEBUG Library

Color-coded logging system for FPVGate serial output.

## Features

- **Ring buffer** for storing last 100 log entries
- **Color-coded prefixes** for serial monitor visualization
- **Backwards compatible** with existing `DEBUG()` macro
- **Timestamp support** (milliseconds since boot)

## Usage

### Log Levels

```cpp
#include "debug.h"

// Error messages (Red in serial monitor)
LOG_ERROR("Failed to initialize SD card\n");
LOG_ERROR("Connection failed: %d\n", errorCode);

// Warning messages (Orange in serial monitor)
LOG_WARN("Battery voltage low: %.2fV\n", voltage);
LOG_WARN("WiFi connection lost, reconnecting\n");

// Info messages (Blue in serial monitor)
LOG_INFO("WiFi SSID: %s\n", ssid);
LOG_INFO("IP address: %s\n", ipAddress);

// Success messages (Green in serial monitor)
LOG_SUCCESS("WiFi connected successfully!\n");
LOG_SUCCESS("SD card initialized\n");

// Debug messages (Gray in serial monitor)
LOG_DEBUG("RSSI value: %d\n", rssi);
LOG_DEBUG("Free heap: %d bytes\n", freeHeap);

// Legacy macro (no color prefix)
DEBUG("Generic message\n");
```

### Serial Monitor Compatibility

The logging system outputs prefixes that are recognized by modern serial monitors:

| Macro | Prefix | Color |
|-------|--------|-------|
| `LOG_ERROR()` | `[ERROR]` | Red |
| `LOG_WARN()` | `[WARN]` | Orange |
| `LOG_INFO()` | `[INFO]` | Blue |
| `LOG_SUCCESS()` | `[SUCCESS]` | Green |
| `LOG_DEBUG()` | `[DEBUG]` | Gray |
| `DEBUG()` | None | Default |

### Output Format

```
[ERROR] [1234] Failed to connect to WiFi
[WARN] [5678] Battery low: 3.2V
[INFO] [9012] SSID: FPVGate_ABC123
[SUCCESS] [12345] WiFi connected!
[DEBUG] [15678] RSSI: 85
[90123] Legacy debug message
```

Format: `[PREFIX] [TIMESTAMP_MS] Message`

## API Reference

### Macros

```cpp
LOG_ERROR(format, ...)   // Red error messages
LOG_WARN(format, ...)    // Orange warnings
LOG_INFO(format, ...)    // Blue information
LOG_SUCCESS(format, ...) // Green success messages
LOG_DEBUG(format, ...)   // Gray debug output
DEBUG(format, ...)       // Legacy (no color)
```

All macros support `printf`-style formatting.

### DebugLogger Class

```cpp
// Get singleton instance
DebugLogger& logger = DebugLogger::getInstance();

// Log with specific level
logger.logWithLevel(LOG_ERROR, "Error: %d\n", code);

// Get log buffer
const std::vector<LogEntry>& logs = logger.getBuffer();

// Clear log buffer
logger.clear();
```

### LogEntry Structure

```cpp
struct LogEntry {
    unsigned long timestamp;  // Milliseconds since boot
    char message[256];        // Log message (without prefix)
};
```

## Configuration

```cpp
// In debuglogger.h:
#define DEBUG_BUFFER_SIZE 100  // Ring buffer size

// In debug.h:
#define SERIAL_BAUD 460800     // Serial baud rate
```

## Best Practices

1. **Use appropriate levels:**
   - `LOG_ERROR` for critical failures
   - `LOG_WARN` for recoverable issues
   - `LOG_INFO` for status and configuration
   - `LOG_SUCCESS` for successful operations
   - `LOG_DEBUG` for detailed debugging

2. **Always include newlines:**
   ```cpp
   LOG_INFO("WiFi connected\n");  // Correct
   LOG_INFO("WiFi connected");    // Missing \n
   ```

3. **Format efficiently:**
   ```cpp
   LOG_INFO("IP: %s, RSSI: %d\n", ip, rssi);  // Good
   LOG_INFO("IP: ");                           // Avoid multiple
   LOG_INFO("%s", ip);                         // calls per line
   LOG_INFO("\n");
   ```

4. **Keep messages concise:**
   - Maximum 256 characters per message
   - Focus on actionable information

## Example: WiFi Status Logging

```cpp
void connectWiFi() {
    LOG_INFO("Connecting to WiFi network\n");
    
    if (WiFi.begin(ssid, password)) {
        LOG_SUCCESS("WiFi connected successfully!\n");
        LOG_INFO("IP address: %s\n", WiFi.localIP().toString().c_str());
        LOG_INFO("SSID: %s\n", WiFi.SSID().c_str());
    } else {
        LOG_ERROR("WiFi connection failed\n");
        LOG_WARN("Falling back to AP mode\n");
    }
}
```

## Backwards Compatibility

The `DEBUG()` macro remains unchanged for existing code:

```cpp
DEBUG("This works like before\n");  // No prefix, no color
```

To gradually migrate to color-coded logging:

1. Identify log message purpose
2. Replace `DEBUG()` with appropriate `LOG_*()` macro
3. Test output in serial monitor

## Notes

- Serial baud rate is **460800** (configured in `debug.h`)
- Log buffer stores last **100 entries** (ring buffer)
- Timestamps are in **milliseconds** since ESP32 boot
- Color rendering depends on serial monitor support
