#include <Arduino.h>

#include "can.h"
#include "oled.h"
#include "serialcom.h"

#define OLED_ENABLED true
OLED oled;

#define CAN_ENABLED true
CANBus can;

#define SCOM_ENABLED true
serialCom scom;

bool has_serial = false;
unsigned long last_login_ms = 0;
bool fill_waiting_drawn = false;

void setup() {
  Serial.begin(115200);
  Serial.println("[FLATPACK2 CONTROLLER][INFO] Beginning setup...");
  
  if (OLED_ENABLED && !oled.begin()) {
    Serial.println("[FLATPACK2 CONTROLLER][WARNING] OLED failed.");
  }
  
  if (CAN_ENABLED && !can.begin()) {
    Serial.println("[FLATPACK2 CONTROLLER][ERROR] CAN failed. Halting...");
    while (1) delay(500);
  }

}

void loop() {
  if (!CAN_ENABLED) return;
  
  // If CANBus::RX_LOGIN_REQUEST received once - Login every 10 seconds
  if (has_serial && millis() - last_login_ms > 10000) {
    can.sendLogin();
    last_login_ms = millis();
  }
  
  twai_message_t msg;
  if (can.receive(msg)) {
    if (!has_serial && (msg.identifier & CANBus::RX_LOGIN_REQUEST_MASK) == CANBus::RX_LOGIN_REQUEST_BASE) {
      if (can.setSerial(msg)) {
        Serial.println("[CAN][INFO] Received CANBus::RX_LOGIN_REQUEST for the first time");
        has_serial = true;
        
        // Extract and store the PSU ID from the identifier (XX byte)
        uint8_t psu_id = (msg.identifier >> 16) & 0xFF;
        can.setPsuId(psu_id);
        
        if (OLED_ENABLED) oled.fill_static();
        last_login_ms = millis();
        can.sendLogin();
      } else {
        Serial.println("[MAIN][WARN] Login Request received but failed to get serial.");
      }
    } else if (has_serial) {
      can.handleMessage(msg);
    }
  }
  
  if (OLED_ENABLED && !has_serial && !fill_waiting_drawn) {
    oled.fill_waiting();
    fill_waiting_drawn = true;
  }

  if (!SCOM_ENABLED) {
    cmdRx cmd;
    if (scom.receive(cmd) && cmd.valid) {
      switch(cmd.type) {
        case ctfpType::SET_OPERATION:
          can.sendLogin(); delay(100);
          can.setOperatingParams(cmd.current, cmd.voltage, cmd.protection);
          delay(100); can.setDefaultVoltage(cmd.voltage);
          break;
        case ctfpType::DEFAULT_VOLTAGE:
          can.sendLogin(); delay(100);
          can.setDefaultVoltage(cmd.voltage);
          break;
      }
    }
  }

  delay(100);
}
