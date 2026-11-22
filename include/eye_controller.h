#ifndef EYE_CONTROLLER_H
#define EYE_CONTROLLER_H

#include "eye_types.h"
#include <stdint.h>

class EyeController {
public:
    EyeController();

    void begin(uint32_t nowMs);
    void update(uint32_t nowMs);

    void setEmotion(Emotion emotion);
    void setLook(float offsetX, float offsetY);
    void triggerBlink(uint32_t nowMs);
    void setAutoDemo(bool enabled);

    bool isAutoDemo() const { return autoDemo_; }
    Emotion getCurrentEmotion() const { return currentEmotion_; }
    const DualEyeState& getState() const { return rendered_; }

    static const char* emotionName(Emotion emotion);

private:
    Emotion currentEmotion_;
    DualEyeState current_;
    DualEyeState target_;
    DualEyeState rendered_;

    bool autoDemo_;
    uint32_t lastUpdateMs_;
    uint32_t emotionStartMs_;
    uint32_t nextBlinkMs_;
    uint32_t blinkStartMs_;
    bool blinking_;

    uint32_t nextIdleMoveMs_;
    float idleOffsetX_;
    float idleOffsetY_;
    float manualOffsetX_;
    float manualOffsetY_;
    bool hasManualLook_;

    uint32_t prngState_;

    DualEyeState presetForEmotion(Emotion emotion) const;
    uint32_t emotionDurationMs(Emotion emotion) const;
    uint32_t nextRandom(uint32_t maxVal);
    float randomRange(float minVal, float maxVal);

    static float lerp(float a, float b, float t);
    static void lerpEye(EyeParams& dst, const EyeParams& src, float t);
};

#endif // EYE_CONTROLLER_H
