#include <oled.h>

#define SCREEN_WIDTH   128 // _oled display width, in pixels
#define SCREEN_HEIGHT  64 // _oled display height, in pixels
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
#define DEGREE_CHAR    247

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool OLED::begin() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("[OLED][ERROR] SSD1306 allocation failed"));
    return false;
  }
  Serial.println("[OLED][INFO] SSD1306 allocation successful!");
  return true;
}

void OLED::clear_data() {
  display.fillRect(70,8,50,8, BLACK);
  // IN[8,32] 00[27.32] °C[40,32] 00.00[70,32] V[101,32]      
  // EX[8,42] 00[27,42] °C[40,42] 00.00[70,42] A[101,42]
  // [ INPUT ][19,56] 000[77,56] V[100,56]
  // Clear temperatures
  display.fillRect(27, 32, 12, 18, BLACK);
  // Clear currents
  display.fillRect(70, 32, 30, 18, BLACK);
  // Clear input voltage
  display.fillRect(77, 56, 18, 8, BLACK);
}

void OLED::fill_header(){
  display.clearDisplay();
  // Top Header (Inverted)
  display.setTextColor(BLACK, WHITE);
  display.setCursor(1,0);
  display.println(" FLATPACK CONTROLLER ");
  display.setTextColor(WHITE);
  display.setCursor(12,10);
  display.print("Status:");
  display.drawLine(0, 18, SCREEN_WIDTH - 1, 18, WHITE);
}

void OLED::fill_waiting() {
  fill_header();
  display.setCursor(1, 32);
  display.println("Waiting for Flatpack2");
  display.setCursor(22, 38);
  display.println("Login Request");
  display.display();
}

void OLED::fill_static() {
  fill_header();
  display.setCursor(7, 22);
  display.println("[ TEMP ] [ OUTPUT ]");
  display.setCursor(8, 32);
  display.print("IN:");
  display.setCursor(40, 32);
  display.print((char)DEGREE_CHAR); display.print("C");
  display.setCursor(8, 42);
  display.print("EX:");
  display.setCursor(40, 42);
  display.print((char)DEGREE_CHAR); display.print("C");
  display.setCursor(101, 32);
  display.print(" V");
  display.setCursor(101, 42);
  display.print(" A");
  display.drawLine(0, 53, SCREEN_WIDTH - 1, 53, WHITE);
  display.setCursor(19, 56);
  display.print("[ INPUT ]");
  display.setCursor(100, 56);
  display.print("V");
  display.display();
}

void OLED::update_data(int intake_temp, int exhaust_temp, float output_voltage, float output_current, int input_voltage, String status) {
  OLED::clear_data();
  // IN[8,32] 00[27.32] °C[40,32] 00.00[70,32] V[101,32]      
  // EX[8,42] 00[27,42] °C[40,42] 00.00[70,42] A[101,42]
  // [ INPUT ][19,56] 000[77,56] V[100,56]
  display.setCursor(27,32); display.print(intake_temp);
  display.setCursor(27,42); display.print(exhaust_temp);
  display.setCursor(70,32); display.print(output_voltage);
  display.setCursor(70,42); display.print(output_current);
  display.setCursor(77,56); display.print(input_voltage);
  // [ status ][10,10] Warning[72,10]
  display.setCursor(72,10); display.print(status);
  display.display();
}

