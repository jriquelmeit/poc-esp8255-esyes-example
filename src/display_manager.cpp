#ifdef ARDUINO

#include "display_manager.h"
#include "config.h"
#include <Wire.h>
#include <algorithm>
#include <cmath>

DisplayManager::DisplayManager()
    : leftDisplay_(OLED_I2C_ADDR, LEFT_EYE_SDA, LEFT_EYE_SCL),
      rightDisplay_(OLED_I2C_ADDR, RIGHT_EYE_SDA, RIGHT_EYE_SCL) {}

void DisplayManager::begin() {
    Wire.begin(LEFT_EYE_SDA, LEFT_EYE_SCL);
    leftDisplay_.init();
    leftDisplay_.setContrast(255);
    leftDisplay_.clear();
    leftDisplay_.display();

    Wire.begin(RIGHT_EYE_SDA, RIGHT_EYE_SCL);
    rightDisplay_.init();
    rightDisplay_.setContrast(255);
    rightDisplay_.clear();
    rightDisplay_.display();

    Wire.setClock(I2C_CLOCK_HZ);
}

void DisplayManager::render(const DualEyeState& state) {
    Wire.begin(LEFT_EYE_SDA, LEFT_EYE_SCL);
    Wire.setClock(I2C_CLOCK_HZ);
    renderEye(leftDisplay_, state.left, true);
    leftDisplay_.display();

    Wire.begin(RIGHT_EYE_SDA, RIGHT_EYE_SCL);
    Wire.setClock(I2C_CLOCK_HZ);
    renderEye(rightDisplay_, state.right, false);
    rightDisplay_.display();
}

void DisplayManager::renderEye(OLEDDisplay& display, const EyeParams& eye, bool isLeftEye) {
    display.clear();
    display.setColor(WHITE);

    const float cx = (SCREEN_WIDTH * 0.5f) + eye.offsetX;
    const float cy = (SCREEN_HEIGHT * 0.5f) + eye.offsetY;

    float h = eye.height * eye.openness;
    if (h < 3.0f) {
        h = 3.0f;
    }
    const float w = eye.width;
    float r = eye.cornerRadius;
    const float maxR = std::min(w, h) * 0.5f;
    if (r > maxR) {
        r = maxR;
    }

    const int16_t x = static_cast<int16_t>(std::lround(cx - w * 0.5f));
    const int16_t y = static_cast<int16_t>(std::lround(cy - h * 0.5f));
    const int16_t iw = static_cast<int16_t>(std::lround(w));
    const int16_t ih = static_cast<int16_t>(std::lround(h));
    const int16_t ir = static_cast<int16_t>(std::lround(r));

    fillRoundRect(display, x, y, iw, ih, ir);

    // Carve eyelids and expressions in black
    display.setColor(BLACK);

    // 1. Top angled eyelid (ANGRY / SAD / THINKING)
    if (eye.topInnerDrop > 0.5f || eye.topOuterDrop > 0.5f) {
        const float leftDrop = isLeftEye ? eye.topOuterDrop : eye.topInnerDrop;
        const float rightDrop = isLeftEye ? eye.topInnerDrop : eye.topOuterDrop;

        const int16_t xL = x - 2;
        const int16_t xR = x + iw + 2;
        const int16_t yTop = y - 2;
        const int16_t yL = y + static_cast<int16_t>(std::lround(leftDrop * eye.openness));
        const int16_t yR = y + static_cast<int16_t>(std::lround(rightDrop * eye.openness));

        fillTriangle(display, xL, yTop, xR, yTop, xL, yL);
        fillTriangle(display, xR, yTop, xR, yR, xL, yL);
    }

    // 2. Bottom upward crescent curve (HAPPY / WINK)
    if (eye.bottomCurve > 0.05f) {
        const int16_t curveR = static_cast<int16_t>(std::lround(w * 0.62f));
        const int16_t lift = static_cast<int16_t>(std::lround(eye.bottomCurve * (h * 0.48f)));
        const int16_t circleY = (y + ih) + curveR - lift;
        display.fillCircle(static_cast<int16_t>(std::lround(cx)), circleY, curveR);
    }
}

void DisplayManager::fillRoundRect(OLEDDisplay& d, int16_t x, int16_t y,
                                   int16_t w, int16_t h, int16_t r) {
    if (w <= 0 || h <= 0) {
        return;
    }
    if (r <= 0) {
        d.fillRect(x, y, w, h);
        return;
    }
    if (r * 2 > w) r = w / 2;
    if (r * 2 > h) r = h / 2;

    // Center & side rectangles
    d.fillRect(x + r, y, w - 2 * r, h);
    d.fillRect(x, y + r, r, h - 2 * r);
    d.fillRect(x + w - r, y + r, r, h - 2 * r);

    // Four rounded corners
    d.fillCircle(x + r, y + r, r);
    d.fillCircle(x + w - r - 1, y + r, r);
    d.fillCircle(x + r, y + h - r - 1, r);
    d.fillCircle(x + w - r - 1, y + h - r - 1, r);
}

void DisplayManager::fillTriangle(OLEDDisplay& d, int16_t x0, int16_t y0,
                                  int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    if (y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }
    if (y1 > y2) { std::swap(y1, y2); std::swap(x1, x2); }
    if (y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }

    if (y0 == y2) {
        int16_t minX = std::min({x0, x1, x2});
        int16_t maxX = std::max({x0, x1, x2});
        d.drawHorizontalLine(minX, y0, maxX - minX + 1);
        return;
    }

    const int32_t dx01 = x1 - x0, dy01 = y1 - y0;
    const int32_t dx02 = x2 - x0, dy02 = y2 - y0;
    const int32_t dx12 = x2 - x1, dy12 = y2 - y1;

    int32_t sa = 0;
    int32_t sb = 0;

    int16_t last = (y1 == y2) ? y1 : y1 - 1;
    for (int16_t y = y0; y <= last; ++y) {
        int16_t a = x0 + (dy01 != 0 ? sa / dy01 : 0);
        int16_t b = x0 + (dy02 != 0 ? sb / dy02 : 0);
        sa += dx01;
        sb += dx02;
        if (a > b) std::swap(a, b);
        d.drawHorizontalLine(a, y, b - a + 1);
    }

    sa = dx12 * (last + 1 - y1);
    sb = dx02 * (last + 1 - y0);
    for (int16_t y = last + 1; y <= y2; ++y) {
        int16_t a = x1 + (dy12 != 0 ? sa / dy12 : 0);
        int16_t b = x0 + (dy02 != 0 ? sb / dy02 : 0);
        sa += dx12;
        sb += dx02;
        if (a > b) std::swap(a, b);
        d.drawHorizontalLine(a, y, b - a + 1);
    }
}

#endif // ARDUINO
