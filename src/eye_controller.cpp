#include "eye_controller.h"

namespace {
constexpr uint32_t BLINK_DURATION_MS = 180;
constexpr float LERP_SPEED = 10.0f; // Higher = snappier transitions

EyeParams baseNeutralEye() {
    EyeParams e;
    e.offsetX = 0.0f;
    e.offsetY = 0.0f;
    e.width = 66.0f;
    e.height = 46.0f;
    e.cornerRadius = 14.0f;
    e.openness = 1.0f;
    e.topInnerDrop = 0.0f;
    e.topOuterDrop = 0.0f;
    e.bottomCurve = 0.0f;
    return e;
}
} // namespace

EyeController::EyeController()
    : currentEmotion_(Emotion::NEUTRAL),
      autoDemo_(true),
      lastUpdateMs_(0),
      emotionStartMs_(0),
      nextBlinkMs_(2500),
      blinkStartMs_(0),
      blinking_(false),
      nextIdleMoveMs_(1500),
      idleOffsetX_(0.0f),
      idleOffsetY_(0.0f),
      manualOffsetX_(0.0f),
      manualOffsetY_(0.0f),
      hasManualLook_(false),
      prngState_(0x13579BDFu) {
    current_ = presetForEmotion(Emotion::NEUTRAL);
    target_ = current_;
    rendered_ = current_;
}

void EyeController::begin(uint32_t nowMs) {
    lastUpdateMs_ = nowMs;
    emotionStartMs_ = nowMs;
    nextBlinkMs_ = nowMs + 2200;
    nextIdleMoveMs_ = nowMs + 1400;
    blinking_ = false;
    setEmotion(Emotion::NEUTRAL);
}

void EyeController::setEmotion(Emotion emotion) {
    currentEmotion_ = emotion;
    target_ = presetForEmotion(emotion);
    hasManualLook_ = false;
}

void EyeController::setLook(float offsetX, float offsetY) {
    if (offsetX < -24.0f) offsetX = -24.0f;
    if (offsetX > 24.0f) offsetX = 24.0f;
    if (offsetY < -7.0f) offsetY = -7.0f;
    if (offsetY > 7.0f) offsetY = 7.0f;
    manualOffsetX_ = offsetX;
    manualOffsetY_ = offsetY;
    hasManualLook_ = true;
}

void EyeController::triggerBlink(uint32_t nowMs) {
    if (!blinking_) {
        blinking_ = true;
        blinkStartMs_ = nowMs;
    }
}

void EyeController::setAutoDemo(bool enabled) {
    autoDemo_ = enabled;
}

void EyeController::update(uint32_t nowMs) {
    uint32_t dtMs = nowMs - lastUpdateMs_;
    if (dtMs > 100) {
        dtMs = 100;
    }
    lastUpdateMs_ = nowMs;
    float dtSec = static_cast<float>(dtMs) / 1000.0f;

    // 1. Auto-demo state machine
    if (autoDemo_ && (nowMs - emotionStartMs_ >= emotionDurationMs(currentEmotion_))) {
        uint8_t nextIdx = (static_cast<uint8_t>(currentEmotion_) + 1) %
                          static_cast<uint8_t>(Emotion::COUNT);
        setEmotion(static_cast<Emotion>(nextIdx));
        emotionStartMs_ = nowMs;
        idleOffsetX_ = 0.0f;
        idleOffsetY_ = 0.0f;
    }

    // 2. Idle gaze micro-movements (only in NEUTRAL or SLEEPY-free states)
    if (!hasManualLook_ && nowMs >= nextIdleMoveMs_) {
        if (currentEmotion_ == Emotion::NEUTRAL) {
            idleOffsetX_ = randomRange(-18.0f, 18.0f);
            idleOffsetY_ = randomRange(-5.0f, 5.0f);
        } else if (currentEmotion_ == Emotion::THINKING) {
            idleOffsetX_ = (idleOffsetX_ > 0.0f) ? -14.0f : 14.0f;
            idleOffsetY_ = -6.0f;
        } else {
            idleOffsetX_ = 0.0f;
            idleOffsetY_ = 0.0f;
        }
        nextIdleMoveMs_ = nowMs + 1200 + nextRandom(1800);
    }

    // 3. Automatic natural blinking
    if (nowMs >= nextBlinkMs_ && currentEmotion_ != Emotion::SLEEPY) {
        triggerBlink(nowMs);
        nextBlinkMs_ = nowMs + 2400 + nextRandom(2800);
    }

    // 4. Compute effective target with gaze offsets
    DualEyeState effectiveTarget = target_;
    float gazeX = hasManualLook_ ? manualOffsetX_ : idleOffsetX_;
    float gazeY = hasManualLook_ ? manualOffsetY_ : idleOffsetY_;

    if (currentEmotion_ == Emotion::NEUTRAL || currentEmotion_ == Emotion::THINKING ||
        hasManualLook_) {
        effectiveTarget.left.offsetX = gazeX;
        effectiveTarget.left.offsetY = gazeY;
        effectiveTarget.right.offsetX = gazeX;
        effectiveTarget.right.offsetY = gazeY;
    }

    // 5. Smooth interpolation towards target
    float factor = dtSec * LERP_SPEED;
    if (factor > 1.0f) {
        factor = 1.0f;
    }
    lerpEye(current_.left, effectiveTarget.left, factor);
    lerpEye(current_.right, effectiveTarget.right, factor);

    // 6. Apply blink overlay without corrupting base state
    rendered_ = current_;
    if (blinking_) {
        uint32_t elapsed = nowMs - blinkStartMs_;
        if (elapsed >= BLINK_DURATION_MS) {
            blinking_ = false;
        } else {
            float half = static_cast<float>(BLINK_DURATION_MS) * 0.5f;
            float progress = (elapsed < half)
                                 ? (static_cast<float>(elapsed) / half)
                                 : (1.0f - (static_cast<float>(elapsed) - half) / half);
            float blinkScale = 1.0f - progress;
            if (blinkScale < 0.05f) {
                blinkScale = 0.05f;
            }
            rendered_.left.openness *= blinkScale;
            rendered_.right.openness *= blinkScale;
        }
    }
}

DualEyeState EyeController::presetForEmotion(Emotion emotion) const {
    DualEyeState state;
    state.left = baseNeutralEye();
    state.right = baseNeutralEye();

    switch (emotion) {
        case Emotion::NEUTRAL:
            break;

        case Emotion::HAPPY:
            state.left.height = 48.0f;
            state.left.bottomCurve = 1.0f;
            state.right = state.left;
            break;

        case Emotion::SURPRISED:
            state.left.width = 74.0f;
            state.left.height = 54.0f;
            state.left.cornerRadius = 24.0f;
            state.right = state.left;
            break;

        case Emotion::ANGRY:
            state.left.height = 44.0f;
            state.left.topInnerDrop = 22.0f;
            state.left.topOuterDrop = 3.0f;
            state.right = state.left;
            break;

        case Emotion::SAD:
            state.left.height = 42.0f;
            state.left.offsetY = 3.0f;
            state.left.topInnerDrop = 3.0f;
            state.left.topOuterDrop = 22.0f;
            state.right = state.left;
            break;

        case Emotion::THINKING:
            state.left.height = 36.0f;
            state.left.offsetX = 14.0f;
            state.left.offsetY = -6.0f;
            state.left.topInnerDrop = 5.0f;
            state.left.topOuterDrop = 5.0f;
            state.right = state.left;
            break;

        case Emotion::WINK:
            // Left eye stays open, right eye closes playfully
            state.right.openness = 0.06f;
            state.right.bottomCurve = 0.6f;
            break;

        case Emotion::SLEEPY:
            state.left.openness = 0.12f;
            state.left.offsetY = 4.0f;
            state.right = state.left;
            break;

        default:
            break;
    }

    return state;
}

uint32_t EyeController::emotionDurationMs(Emotion emotion) const {
    switch (emotion) {
        case Emotion::NEUTRAL:
            return 4000;
        case Emotion::HAPPY:
            return 3000;
        case Emotion::SURPRISED:
            return 2500;
        case Emotion::ANGRY:
            return 3000;
        case Emotion::SAD:
            return 3000;
        case Emotion::THINKING:
            return 3000;
        case Emotion::WINK:
            return 1600;
        case Emotion::SLEEPY:
            return 3000;
        default:
            return 3000;
    }
}

const char* EyeController::emotionName(Emotion emotion) {
    switch (emotion) {
        case Emotion::NEUTRAL:
            return "NEUTRAL";
        case Emotion::HAPPY:
            return "HAPPY";
        case Emotion::SURPRISED:
            return "SURPRISED";
        case Emotion::ANGRY:
            return "ANGRY";
        case Emotion::SAD:
            return "SAD";
        case Emotion::THINKING:
            return "THINKING";
        case Emotion::WINK:
            return "WINK";
        case Emotion::SLEEPY:
            return "SLEEPY";
        default:
            return "UNKNOWN";
    }
}

uint32_t EyeController::nextRandom(uint32_t maxVal) {
    if (maxVal == 0) {
        return 0;
    }
    // Xorshift32 PRNG (deterministic & portable for native tests)
    uint32_t x = prngState_;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    prngState_ = x;
    return x % maxVal;
}

float EyeController::randomRange(float minVal, float maxVal) {
    uint32_t r = nextRandom(1000);
    float t = static_cast<float>(r) / 999.0f;
    return minVal + (maxVal - minVal) * t;
}

float EyeController::lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

void EyeController::lerpEye(EyeParams& dst, const EyeParams& src, float t) {
    dst.offsetX = lerp(dst.offsetX, src.offsetX, t);
    dst.offsetY = lerp(dst.offsetY, src.offsetY, t);
    dst.width = lerp(dst.width, src.width, t);
    dst.height = lerp(dst.height, src.height, t);
    dst.cornerRadius = lerp(dst.cornerRadius, src.cornerRadius, t);
    dst.openness = lerp(dst.openness, src.openness, t);
    dst.topInnerDrop = lerp(dst.topInnerDrop, src.topInnerDrop, t);
    dst.topOuterDrop = lerp(dst.topOuterDrop, src.topOuterDrop, t);
    dst.bottomCurve = lerp(dst.bottomCurve, src.bottomCurve, t);
}
