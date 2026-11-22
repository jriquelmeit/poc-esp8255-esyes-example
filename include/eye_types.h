#ifndef EYE_TYPES_H
#define EYE_TYPES_H

#include <stdint.h>

enum class Emotion : uint8_t {
    NEUTRAL = 0,
    HAPPY,
    SURPRISED,
    ANGRY,
    SAD,
    THINKING,
    WINK,
    SLEEPY,
    COUNT
};

struct EyeParams {
    float offsetX;       // Horizontal gaze offset in pixels (-24..+24)
    float offsetY;       // Vertical gaze offset in pixels (-7..+7)
    float width;         // Base eye width in pixels
    float height;        // Base eye height in pixels
    float cornerRadius;  // Corner radius in pixels
    float openness;      // 0.0 (closed) .. 1.0 (fully open)
    float topInnerDrop;  // Top eyelid drop on inner corner (px, angry > 0)
    float topOuterDrop;  // Top eyelid drop on outer corner (px, sad > 0)
    float bottomCurve;   // 0.0 (flat) .. 1.0 (happy crescent curve)
};

struct DualEyeState {
    EyeParams left;
    EyeParams right;
};

#endif // EYE_TYPES_H
