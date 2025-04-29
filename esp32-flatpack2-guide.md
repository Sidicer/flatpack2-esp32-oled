# Flatpack2 CAN Controller using ESP32

## Hardware Components

- ESP32 Dev Board
- CAN transceiver (either one): 
- - SN65HVD230 *(3.3V)* 
- - TJA1051T/3 *(with 5V booster)*
- 2x 120Ω resistors (bus termination)
- Length of 2 wire twisted cable (pair of CAT5E works great)

## Detailed Connections

### 1. Schematic

```
+- (5V) USB Power
+-----------------+      +------------------+      +---------------------+
|      ESP32      |      | SN65HVD230 Board |      |   Flatpack2 PSU     |
|                 |      |                  |      |                     |
|                 |      |                  |      |              AC (L) |
|                 |      |                  |      |              AC (N) |
|           3.3V  | ---- | 3.3V    +- CAN_H | ---- | CAN_H -+            |
|                 |      |        120Ω      |      |       120Ω          |
|                 |      |         +- CAN_L | ---- | CAN_L -+            |
|   GPIO 17 (TX2) | ---- | TX               |      |                     |
|   GPIO 16 (RX2) | ---- | RX               |      |                     |
|                 |      |                  |      |             DC +48V |
|            GND  | ---- | GND              |      |             DC -48V |
|                 |      |                  |      |                     |
+-----------------+      +------------------+      +---------------------+
```

> [!IMPORTANT]
> 2x **120Ω resistors**: On each end of CAN twisted cable connecting CAN_H to CAN_L

> [!TIP]
> If using **TJA1051T/3** *(Adafruit CAN PAL)* connections stay the same, just connect **SLNT** *(S)* pin to **GND**

> [!WARNING]
> Please note that -48V is not connected to tranceiver or ESP32!


## ESP-IDF Configuration

```cpp
#define TWAI_TX_PIN GPIO_NUM_17
#define TWAI_RX_PIN GPIO_NUM_16

twai_general_config_t g_config = {
   .mode = TWAI_MODE_NORMAL,
   .tx_io = TWAI_TX_PIN,
   .rx_io = TWAI_RX_PIN,
   .clkout_io = TWAI_IO_UNUSED,
   .bus_off_io = TWAI_IO_UNUSED,
   .tx_queue_len = 10,
   .rx_queue_len = 10,
   .alerts_enabled = TWAI_ALERT_NONE,
   .clkout_divider = 0
};

twai_timing_config_t t_config = TWAI_TIMING_CONFIG_125KBITS();
twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
```

## Basic Communication Protocol

The Flatpack2 CAN protocol:
- Uses 125kbit/s baud rate
- Uses extended ID field
- Requires login message every 15 seconds
- Sends status updates with voltage and current data

### Example Messages

1. **Login to PSU (TX)**: `0x050048XX` where XX = ID * 4
2. **Status Updates (RX)**: `0x05XX40YY` with current, voltage, temperature
3. **Set Default Voltage (TX)**: `0x05XX9C00`

Refer to the complete protocol documentation for detailed message structures.

## Basic Implementation Steps

1. Initialize ESP32 TWAI (CAN) driver
2. Send login message to PSU
3. Read status messages
4. Process alarms/warnings if necessary
5. Send control commands as needed
6. Re-send login message every ~10 seconds (before 15s timeout)

## Notes

- The Flatpack2 will log out automatically after 15 seconds without a login message
- All voltage values are in centivolts (48.52V = 4852)
- All current values are in deciamps (21.2A = 212)
