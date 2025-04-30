#ifndef SERIALCOM_H
#define SERIALCOM_H

#include <Arduino.h>

enum class ctfpType {
    NONE,
    SET_OPERATION,
    DEFAULT_VOLTAGE
};

struct cmdRx {
    ctfpType type;
    int voltage;
    int current;
    int protection;
    bool valid;
};

class serialCom {
public:
    static const char endChar = '>';
    static const String cmdStart;
    
    bool DEBUG = false;
    bool receive(cmdRx& cmd);
    String ctfpToString(ctfpType type);

private:
    bool isCommand(const String data);
    bool parseCommand(const String data, cmdRx& cmd);
};

#endif // SERIALCOM_H
