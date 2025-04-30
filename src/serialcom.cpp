#include <serialcom.h>

const String serialCom::cmdStart = "<CTFP,";

bool serialCom::receive(cmdRx& cmd) {
    if (Serial.available() > 0) {
        String rawdata = Serial.readStringUntil(this->endChar);
        if (this->DEBUG) Serial.println("[DEBUG]: Raw RX: " + rawdata);
        if (this->isCommand(rawdata)) {
            return this->parseCommand(rawdata, cmd);
        }
    }
    return false;
}

bool serialCom::isCommand(const String data) {
    return data.startsWith(this->cmdStart);
}

bool serialCom::parseCommand(const String data, cmdRx& cmd) {
    // Expecting two variants:
    // <CTFP,SET,0000,000,0000> Set current operation {voltage,current,over-voltage}
    // <CTFP,DEF,0000> Set default voltage {voltage}
    String commandOnly = data.substring(this->cmdStart.length(), data.length());
    if (this->DEBUG) Serial.println("[DEBUG] Command Received: " + commandOnly);
    
    // SET,4800,100,5000
    int firstComma = commandOnly.indexOf(',');
    int secondComma = commandOnly.indexOf(',', firstComma + 1);
    int thirdComma = commandOnly.indexOf(',', secondComma + 1);

    String commandType = commandOnly.substring(0,firstComma);
    if (this->DEBUG) Serial.println("[DEBUG] commandType: " + commandType);

    if (commandType == "SET") {
        cmd.type = ctfpType::SET_OPERATION;
        if (this->DEBUG) Serial.println("[DEBUG] cmd.type set to: " + this->ctfpToString(cmd.type));
        if (secondComma == -1 || thirdComma == -1 || commandOnly.length() != 17) {
            if (this->DEBUG) Serial.println("[DEBUG] Malformed SET command");
            cmd.valid = false;
            return false;
        }
        cmd.voltage = commandOnly.substring(firstComma + 1, secondComma).toInt();
        cmd.current = commandOnly.substring(secondComma + 1, thirdComma).toInt();
        cmd.protection = commandOnly.substring(thirdComma + 1).toInt();
        cmd.valid = true;
    } else if (commandType == "DEF") {
        cmd.type = ctfpType::DEFAULT_VOLTAGE;
        if (this->DEBUG) Serial.println("[DEBUG] cmd.type set to: " + this->ctfpToString(cmd.type));

        if (commandOnly.length() != 8) {
            if (this->DEBUG) Serial.println("[DEBUG] Malformed DEF command");
            cmd.valid = false;
            return false;
        }
        cmd.voltage = commandOnly.substring(firstComma + 1).toInt();
        cmd.current = 0;
        cmd.protection = 0;
        cmd.valid = true;
    } else {
        cmd.type = ctfpType::NONE;
        if (this->DEBUG) Serial.println("[DEBUG] cmd.type set to: " + this->ctfpToString(cmd.type));
        cmd.valid = false;
        return false;
    }

    return true;
}

String serialCom::ctfpToString(ctfpType type) {
    switch(type) {
        case ctfpType::SET_OPERATION:   return "SET_OPERATION";
        case ctfpType::DEFAULT_VOLTAGE: return "DEFAULT_VOLTAGE";
        default:                        return "NONE";
    }
}
