# Flatpack2 CAN Controller using ESP32

[!image](oled-status-screen.jpg)

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

### 2. Setup

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

## Commands via Serial

Current project has the ability to control flatpack2 via serial:

```sh
echo -ne '<CTFP,SET,4320,100,5800>' >/dev/ttyUSB0
echo -ne '<CTFP,DEF,5410>' >/dev/ttyUSB0
```

- `<CTFP,` Start of message
- `SET/DEF`: SET_OPERATION/DEFAULT_VOLTAGE
- - `SET` `VOLTAGE,CURRENT,PROTECTION`
- - `DEF` `VOLTAGE`
- `>` End of message

## Enable/Disable parts of code

`main.cpp` 
```cpp
#define OLED_ENABLED true # Enable/Disable OLED screen
#define CAN_ENABLED true  # Enable/Disable CAN (loop() doesn't run)
#define SCOM_ENABLED true # Enable/Disable Commands via Serial 
```

### 3. Sources

Wouldn't have done anything without:
- https://github.com/the6p4c/Flatpack2
- https://github.com/taHC81/Eltek-Flatpack2-ESPhome
- https://openinverter.org/forum/viewtopic.php?t=1351
