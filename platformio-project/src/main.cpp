#include <Arduino.h>

#include "can.h"
#include "oled.h"

#define OLED_ENABLED true
OLED oled;

#define CAN_ENABLED true
CANBus can;
bool has_serial = false;
unsigned long last_login_ms = 0;

#define VOLTAGE_SET_PIN GPIO_NUM_4
#define TARGET_VOLTAGE_CV 5400
int VOLTAGE_SET_PIN_STATE = HIGH;

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
  
  pinMode(VOLTAGE_SET_PIN, INPUT_PULLUP);
  Serial.print("[FLATPACK2 CONTROLLER][INFO] Configured Voltage Set Pin (GPIO");
  Serial.print(VOLTAGE_SET_PIN);
  Serial.println(") with internal pull-up.");
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
  
  bool fill_waiting_drawn = false;
  if (OLED_ENABLED && !has_serial && !fill_waiting_drawn) {
    oled.fill_waiting();
    fill_waiting_drawn = true;
  }
  
  int pin_reading = digitalRead(VOLTAGE_SET_PIN); 
  if (pin_reading != VOLTAGE_SET_PIN_STATE) {
    VOLTAGE_SET_PIN_STATE = pin_reading;
    
    if (VOLTAGE_SET_PIN_STATE == LOW) {
      Serial.print("[MAIN][INFO] Voltage Set Pin (GPIO");
      Serial.print(VOLTAGE_SET_PIN);
      Serial.println(") grounded. Sending Set Voltage command.");
      
      if (CAN_ENABLED && has_serial) {
        // Send the set voltage command via CAN
        // (not later than 5 seconds after loggin in)
        can.sendLogin();
        // can.setDefaultVoltage(TARGET_VOLTAGE_CV);
        can.setOperatingParams(100, 4800, 5400);
      } else if (!CAN_ENABLED) {
        Serial.println("[MAIN][WARN] Cannot send Set Voltage: CAN is disabled.");
      } else { // CAN enabled but no serial yet
        Serial.println("[MAIN][WARN] Cannot send Set Voltage: PSU Serial/ID not yet known.");
      }
    }
  }
  
  delay(100);
}
