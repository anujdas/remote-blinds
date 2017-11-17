#include "Ui.h"

Ui::Ui(SSD1306* display) {
  this->display = display;
}

void Ui::init() {
  display->init();
  display->clear();
  display->setColor(WHITE);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0, 0, "Connecting...");
  display->display();
}
