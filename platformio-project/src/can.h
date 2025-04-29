#ifndef CAN_H
#define CAN_H

#include <Arduino.h>
#include <driver/twai.h>

class CANBus {
public:
    static constexpr uint32_t RX_LOGIN_REQUEST_MASK   = 0xFF00FFFF;
    static constexpr uint32_t RX_LOGIN_REQUEST_BASE   = 0x05004400;
    
    static constexpr uint32_t RX_STATUS_MASK          = 0xFF00FF00;
    static constexpr uint32_t RX_STATUS_BASE          = 0x05004000;

    static constexpr uint32_t TX_LOGIN_BASE           = 0x05004804;
    static constexpr uint32_t TX_DEFAULT_VOLTAGE_BASE = 0x05009C00;
    static constexpr uint32_t TX_SET_OPERATING_PARAMS = 0x05FF4004; 

    bool begin();
    bool receive(twai_message_t& msg);

    bool setSerial(const twai_message_t& msg);
    bool setPsuId(uint8_t id);
    void sendLogin();

    void setOperatingParams(uint16_t current_da, uint16_t voltage_cv, uint16_t ovp_cv);
    void setDefaultVoltage(const uint16_t voltage_cv);
    void handleMessage(const twai_message_t& msg);

private:
    uint8_t psu_id = 0;
    uint8_t serial_number[6];
    void handleStatus(const twai_message_t& msg);
};

#endif // CAN_H
