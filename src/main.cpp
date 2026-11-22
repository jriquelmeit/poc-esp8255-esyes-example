#ifdef ARDUINO

#include <Arduino.h>
#include "config.h"
#include "display_manager.h"
#include "eye_controller.h"

DisplayManager displayManager;
EyeController eyeController;

static uint32_t lastFrameMs = 0;
static Emotion lastLoggedEmotion = Emotion::COUNT;

void printHelp() {
    Serial.println(F("\n=== ESP8266 Dual OLED Eyes POC ==="));
    Serial.println(F("Comandos Serial:"));
    Serial.println(F("  0: NEUTRAL   1: HAPPY     2: SURPRISED  3: ANGRY"));
    Serial.println(F("  4: SAD       5: THINKING  6: WINK       7: SLEEPY"));
    Serial.println(F("  a: Reanudar modo automatico (demo)"));
    Serial.println(F("  b: Parpadeo manual (blink)"));
    Serial.println(F("==================================\n"));
}

void handleSerialCommands(uint32_t nowMs) {
    while (Serial.available() > 0) {
        char c = static_cast<char>(Serial.read());
        if (c >= '0' && c <= '7') {
            Emotion e = static_cast<Emotion>(c - '0');
            eyeController.setAutoDemo(false);
            eyeController.setEmotion(e);
            Serial.print(F("[Manual] Emocion -> "));
            Serial.println(EyeController::emotionName(e));
        } else if (c == 'a' || c == 'A') {
            eyeController.setAutoDemo(true);
            Serial.println(F("[Auto] Modo demo automatico activado"));
        } else if (c == 'b' || c == 'B') {
            eyeController.triggerBlink(nowMs);
        } else if (c == 'h' || c == '?') {
            printHelp();
        }
    }
}

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(100);

    displayManager.begin();
    eyeController.begin(millis());

    printHelp();
}

void loop() {
    uint32_t nowMs = millis();
    handleSerialCommands(nowMs);

    if (nowMs - lastFrameMs >= FRAME_INTERVAL_MS) {
        lastFrameMs = nowMs;

        eyeController.update(nowMs);
        displayManager.render(eyeController.getState());

        Emotion current = eyeController.getCurrentEmotion();
        if (current != lastLoggedEmotion) {
            lastLoggedEmotion = current;
            Serial.print(F("[Estado] "));
            Serial.println(EyeController::emotionName(current));
        }
    }
}

#endif // ARDUINO
