#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "eye_types.h"

#ifdef ARDUINO
#include <SSD1306Wire.h>

class DisplayManager {
public:
    DisplayManager();

    void begin();
    void render(const DualEyeState& state);

private:
    SSD1306Wire leftDisplay_;
    SSD1306Wire rightDisplay_;

    void renderEye(OLEDDisplay& display, const EyeParams& eye, bool isLeftEye);

    static void fillRoundRect(OLEDDisplay& d, int16_t x, int16_t y,
                              int16_t w, int16_t h, int16_t r);
    static void fillTriangle(OLEDDisplay& d, int16_t x0, int16_t y0,
                             int16_t x1, int16_t y1, int16_t x2, int16_t y2);
};
#endif // ARDUINO

#endif // DISPLAY_MANAGER_H
