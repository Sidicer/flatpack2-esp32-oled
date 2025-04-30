#include "can.h"
#include "oled.h"

#define TWAI_TX_PIN GPIO_NUM_17
#define TWAI_RX_PIN GPIO_NUM_16

OLED oled_can;

bool CANBus::begin() {
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
  
  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    Serial.println("[CAN][ERROR] Failed to install CAN driver");
    return false;
  }
  
  if (twai_start() != ESP_OK) {
    Serial.println("[CAN][ERROR] Failed to start CAN driver");
    return false;
  }
  
  Serial.println("[CAN][INFO] Driver initialized successfully");
  return true;
}

bool CANBus::receive(twai_message_t& msg) {
  return twai_receive(&msg, pdMS_TO_TICKS(1000)) == ESP_OK;
}

bool CANBus::setSerial(const twai_message_t& msg) {
  if (msg.data_length_code < 6) return false;
  for (int i = 0; i < 6; ++i) this->serial_number[i] = msg.data[i];
  return true;
}

bool CANBus::setPsuId(uint8_t id) {
  if (id >= 0x01 && id <= 0x3F) { // Validate ID range
    this->psu_id = id;
    Serial.print("[CAN][INFO] PSU ID set to: 0x");
    Serial.println(id, HEX);
    return true;
  } else {
    Serial.print("[CAN][WARN] Invalid PSU ID received: 0x");
    Serial.println(id, HEX);
    return false;
  }
}

void CANBus::sendLogin() {
  if (this->psu_id == 0) {
    Serial.println("[CAN][ERROR] Cannot send login: PSU ID not set.");
    return;
  }
  
  twai_message_t msg{};
  msg.identifier = TX_LOGIN_BASE | (this->psu_id << 2);  // XX is the ID byte
  msg.flags = TWAI_MSG_FLAG_EXTD;
  msg.data_length_code = 8;
  
  for (int i = 0; i < 6; ++i) msg.data[i] = this->serial_number[i];
  msg.data[6] = msg.data[7] = 0;
  
  if (twai_transmit(&msg, pdMS_TO_TICKS(100)) == ESP_OK) {
    Serial.print("[CAN][INFO] Login message sent to ID 0x");
    Serial.println(this->psu_id, HEX);
  } else {
    Serial.print("[CAN][ERROR] Login message failed for ID 0x");
    Serial.println(this->psu_id, HEX);
  }
}

void CANBus::setOperatingParams(uint16_t current_da, uint16_t voltage_cv, uint16_t ovp_cv) {
  if (this->psu_id == 0) {
    Serial.println("[CAN][ERROR] Cannot send login: PSU ID not set.");
    return;
  }

  if (voltage_cv > this->DC_MAX_VOLTAGE || voltage_cv < this->DC_MIN_VOLTAGE) {
    Serial.println("[CAN][ERROR] Voltage out of bounds (43.20V - 58.00V)!");
    return;
  }

  if (current_da > this->DC_MAX_CURRENT) {
    current_da = DC_MAX_CURRENT;
    Serial.println("[CAN][ERROR] Current set too high. Lowering it to 62.5A!");
  }

  if (ovp_cv > this->DC_MAX_VOLTAGE) {
    ovp_cv = DC_MAX_VOLTAGE;
    Serial.println("[CAN][ERROR] Over-voltage protection set too high. Lowering it to 58.00V!");
  }

  twai_message_t msg{};
  msg.identifier = TX_SET_OPERATING_PARAMS; // Use the fixed broadcast ID
  msg.flags = TWAI_MSG_FLAG_EXTD;
  msg.data_length_code = 8;

  // Pack data bytes (Little Endian)
  // Bytes 0-1: Current limit (dA)
  msg.data[0] = current_da & 0xFF;
  msg.data[1] = (current_da >> 8) & 0xFF;
  // Bytes 2-3: Voltage (cV)
  msg.data[2] = voltage_cv & 0xFF;
  msg.data[3] = (voltage_cv >> 8) & 0xFF;
  // Bytes 4-5: Voltage (cV) - Repeated
  msg.data[4] = voltage_cv & 0xFF;
  msg.data[5] = (voltage_cv >> 8) & 0xFF;
  // Bytes 6-7: OVP level (cV)
  msg.data[6] = ovp_cv & 0xFF;
  msg.data[7] = (ovp_cv >> 8) & 0xFF;

  // Transmit the message
  if (twai_transmit(&msg, pdMS_TO_TICKS(100)) == ESP_OK) {
    Serial.print("[CAN][INFO] Set Operating Params message sent (ID: 0x");
    Serial.print(TX_SET_OPERATING_PARAMS, HEX);
    Serial.print(", V: "); Serial.print(voltage_cv / 100.0);
    Serial.print("V, I: "); Serial.print(current_da / 10.0);
    Serial.print("A, OVP: "); Serial.print(ovp_cv / 100.0);
    Serial.println("V)");
  } else {
    Serial.print("[CAN][ERROR] Set Operating Params message failed (ID: 0x");
    Serial.print(TX_SET_OPERATING_PARAMS, HEX);
    Serial.println(")");
  }

}

void CANBus::setDefaultVoltage(const uint16_t voltage_cv) {
  if (this->psu_id == 0) {
    Serial.println("[CAN][ERROR] Cannot send login: PSU ID not set.");
    return;
  }

  if (voltage_cv > this->DC_MAX_VOLTAGE || voltage_cv < this->DC_MIN_VOLTAGE) {
    Serial.println("[CAN][ERROR] Voltage out of bounds (43.20V - 58.00V)!");
    return;
  }

  twai_message_t msg{};
  msg.identifier = TX_DEFAULT_VOLTAGE_BASE | (this->psu_id << 8); // XX is the ID byte
  msg.flags = TWAI_MSG_FLAG_EXTD;
  msg.data_length_code = 5;
  
  // Populate data bytes according to Protocol.md
  msg.data[0] = 0x29;
  msg.data[1] = 0x15;
  msg.data[2] = 0x00;
  // Voltage is little-endian (LSB first)
  msg.data[3] = voltage_cv & 0xFF;        // Low byte
  msg.data[4] = (voltage_cv >> 8) & 0xFF; // High byte

  // Transmit the message with a 100ms timeout
  if (twai_transmit(&msg, pdMS_TO_TICKS(100)) == ESP_OK) {
    Serial.print("[CAN][INFO] Set Default Voltage message sent to ID 0x");
    Serial.print(this->psu_id, HEX);
    Serial.print(" (Voltage: ");
    Serial.print(voltage_cv / 100.0);
    Serial.println(" V)");
  } else {
    Serial.print("[CAN][ERROR] Set Default Voltage message failed for ID 0x");
    Serial.println(this->psu_id, HEX);
  }
  
}

void CANBus::handleMessage(const twai_message_t& msg) {
  // Login logic was moved to main.cpp loop()
  if ((msg.identifier & RX_STATUS_MASK) == RX_STATUS_BASE) {
    handleStatus(msg);
  }
}

void CANBus::handleStatus(const twai_message_t& msg) {
  uint8_t status_code = msg.identifier & 0xFF;
  
  int intake_temp = msg.data[0];
  float output_current = (msg.data[2] << 8 | msg.data[1]) / 10.0;
  float output_voltage = (msg.data[4] << 8 | msg.data[3]) / 100.0;
  int input_voltage = msg.data[6] << 8 | msg.data[5];
  int exhaust_temp = msg.data[7];
  String current_status = "Unknown";
  
  switch (status_code) {
    case 0x04: current_status = "Normal"; break;
    case 0x08: current_status = "Warning"; break;
    case 0x0C: current_status = "Alarm"; break;
    case 0x10: current_status = "Walk in"; break;
    default:
    Serial.print("[CAN][ERROR] Received unknown status code: 0x");
    Serial.println(status_code, HEX);
    return;
  }
  
  oled_can.update_data(intake_temp, exhaust_temp, output_voltage, output_current, input_voltage, current_status);
}
