#ifndef OLED_H
#define OLED_H

#include <Adafruit_SSD1306.h>

class OLED {
public:
    bool begin();
    void fill_static();
    void fill_waiting();
    void update_data(int intake_temp, int exhaust_temp, float output_voltage, float output_current, int input_voltage, String status);
    void clear_data();

private:
    void fill_header();
};

#endif // OLED_H
