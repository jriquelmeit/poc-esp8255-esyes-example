#include <unity.h>
#include "eye_controller.h"

void setUp() {}
void tearDown() {}

void test_initial_state_is_neutral() {
    EyeController ctrl;
    ctrl.begin(0);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Emotion::NEUTRAL),
                            static_cast<uint8_t>(ctrl.getCurrentEmotion()));
    TEST_ASSERT_TRUE(ctrl.isAutoDemo());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, ctrl.getState().left.openness);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, ctrl.getState().right.openness);
}

void test_smooth_transition_to_surprised() {
    EyeController ctrl;
    ctrl.begin(0);
    ctrl.setAutoDemo(false);
    ctrl.setEmotion(Emotion::SURPRISED);

    // Step 400 ms in 25 ms increments
    for (uint32_t t = 25; t <= 600; t += 25) {
        ctrl.update(t);
    }

    TEST_ASSERT_FLOAT_WITHIN(1.0f, 74.0f, ctrl.getState().left.width);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 54.0f, ctrl.getState().left.height);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 74.0f, ctrl.getState().right.width);
}

void test_asymmetric_wink_state() {
    EyeController ctrl;
    ctrl.begin(0);
    ctrl.setAutoDemo(false);
    ctrl.setEmotion(Emotion::WINK);

    for (uint32_t t = 25; t <= 800; t += 25) {
        ctrl.update(t);
    }

    // Left eye open, right eye closed
    TEST_ASSERT_GREATER_THAN_FLOAT(0.85f, ctrl.getState().left.openness);
    TEST_ASSERT_LESS_THAN_FLOAT(0.15f, ctrl.getState().right.openness);
}

void test_blink_closes_and_reopens() {
    EyeController ctrl;
    ctrl.begin(0);
    ctrl.setAutoDemo(false);
    ctrl.setEmotion(Emotion::NEUTRAL);

    ctrl.triggerBlink(100);
    ctrl.update(190); // Mid-blink (90 ms into 180 ms blink)
    TEST_ASSERT_LESS_THAN_FLOAT(0.2f, ctrl.getState().left.openness);
    TEST_ASSERT_LESS_THAN_FLOAT(0.2f, ctrl.getState().right.openness);

    ctrl.update(300); // After blink finishes
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 1.0f, ctrl.getState().left.openness);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 1.0f, ctrl.getState().right.openness);
}

void test_auto_demo_cycles_emotions() {
    EyeController ctrl;
    ctrl.begin(0);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Emotion::NEUTRAL),
                            static_cast<uint8_t>(ctrl.getCurrentEmotion()));

    // Advance past NEUTRAL duration (4000 ms)
    ctrl.update(4100);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Emotion::HAPPY),
                            static_cast<uint8_t>(ctrl.getCurrentEmotion()));
}

void test_manual_look_clamping() {
    EyeController ctrl;
    ctrl.begin(0);
    ctrl.setAutoDemo(false);
    ctrl.setLook(100.0f, -50.0f);

    for (uint32_t t = 25; t <= 1000; t += 25) {
        ctrl.update(t);
    }

    TEST_ASSERT_FLOAT_WITHIN(0.5f, 24.0f, ctrl.getState().left.offsetX);
    TEST_ASSERT_FLOAT_WITHIN(0.5f, -7.0f, ctrl.getState().left.offsetY);
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 24.0f, ctrl.getState().right.offsetX);
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_initial_state_is_neutral);
    RUN_TEST(test_smooth_transition_to_surprised);
    RUN_TEST(test_asymmetric_wink_state);
    RUN_TEST(test_blink_closes_and_reopens);
    RUN_TEST(test_auto_demo_cycles_emotions);
    RUN_TEST(test_manual_look_clamping);
    return UNITY_END();
}
