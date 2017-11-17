#ifndef Ui_h
#define Ui_h

#include <Arduino.h>
#include <SSD1306.h>

class Ui {
  public:
    Ui(SSD1306* display);

    void init();

  private:
    SSD1306* display;
};

#endif
