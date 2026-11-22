#ifndef CONFIG_H
#define CONFIG_H

// =============================================================================
// Hardware Configuration - NodeMCU v3 (ESP8266) + Dual 0.96" OLED (gm009606v4)
// =============================================================================

// Left Eye OLED (Bus I2C 1)
#define LEFT_EYE_SDA 4   // D2 (GPIO4)
#define LEFT_EYE_SCL 5   // D1 (GPIO5)

// Right Eye OLED (Bus I2C 2)
#define RIGHT_EYE_SDA 12 // D6 (GPIO12)
#define RIGHT_EYE_SCL 14 // D5 (GPIO14)

// Both gm009606v4 modules share default I2C address 0x3C
#define OLED_I2C_ADDR 0x3C

// I2C clock frequency (Hz). 400000 = safe standard fast mode; 700000 = high FPS
#define I2C_CLOCK_HZ 700000

// Display dimensions (128x64 per eye)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Animation timing
#define TARGET_FPS 40
#define FRAME_INTERVAL_MS (1000 / TARGET_FPS)

// Serial monitor baud rate
#define SERIAL_BAUD 115200

#endif // CONFIG_H
