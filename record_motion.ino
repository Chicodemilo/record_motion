#include <Arduino.h>
#include <FastLED.h>

// Pin Definitions For NodeMCU v3!!
#define MOTION_PIN 4 // GPIO4 (D2)
#define BUTTON1_PIN 5 // GPIO5 (D1)
#define BUTTON2_PIN 0 // GPIO0 (D3)
#define LED_PIN 15 // GPIO8 (D8)
#define ONBOARD_LED_PIN LED_BUILTIN // Onboard LED

// LED Configuration
#define NUM_LEDS 5
#define COLOR_ORDER GRB
#define LED_TYPE WS2812B

// Button Debounce Configuration
#define DEBOUNCE_DELAY 100 // Debounce delay in milliseconds

CRGB leds[NUM_LEDS];
int ledMode = 1;
bool motionDetection = true;
unsigned long lastMotionTime;
bool ledBlinking = false;


// Button variables
unsigned long lastButton1Press = 0;
unsigned long lastButton2Press = 0;
int button1State = HIGH;
int button2State = HIGH;
int lastButton1State = HIGH;
int lastButton2State = HIGH;
unsigned long onboardLedBlinkTimer = 0;
bool motionDetected = false; // Add this variable to track if motion has been detected initially



void setup() {
  Serial.begin(9600);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(255);

  pinMode(MOTION_PIN, INPUT);
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);

  //Turn off all leds
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void loop() {
  // Handle onboard LED based on motion detection mode
  if (digitalRead(MOTION_PIN) == HIGH) {
    lastMotionTime = millis();
    if (!motionDetected && motionDetection) {
      motionDetected = true;
      fadeLEDs();
    }
  } else if (motionDetected && millis() - lastMotionTime >= 15000 && motionDetection) {
    motionDetected = false;
    fadeOutLEDs();
  }

  // Debouncing for button 1
  int currentButton1State = digitalRead(BUTTON1_PIN);
  if (currentButton1State != lastButton1State && millis() - lastButton1Press > DEBOUNCE_DELAY) {
    if (currentButton1State == LOW) {
      button1Pressed();
    }
    lastButton1Press = millis();
  }
  lastButton1State = currentButton1State;

  // Debouncing for button 2
  int currentButton2State = digitalRead(BUTTON2_PIN);
  if (currentButton2State != lastButton2State && millis() - lastButton2Press > DEBOUNCE_DELAY) {
    if (currentButton2State == LOW) {
      button2Pressed();
    }
    lastButton2Press = millis();
  }
  lastButton2State = currentButton2State;


    switch (ledMode) {
      case 1:
        mode1();
        break;
      case 2:
        mode2();
        break;
      case 3:
        mode3();
        break;
      case 4:
        mode4();
        break;
      case 5:
        mode5();
        break;
    }

}


void button1Pressed() {
  motionDetection = !motionDetection;
  if (motionDetection) {
    digitalWrite(ONBOARD_LED_PIN, LOW);
  } else {
    blinkOnboardLed(2);
  }
}


void button2Pressed() {
  ledMode++;
  if (ledMode > 5) {
    ledMode = 1;
  }
  // Blink the onboard LED based on the selected mode
  blinkOnboardLed(ledMode);
}


void blinkOnboardLed(int times) {
  ledBlinking = true;
  for (int i = 0; i < times; i++) {
    digitalWrite(ONBOARD_LED_PIN, HIGH);
    delay(200);
    digitalWrite(ONBOARD_LED_PIN, LOW);
    delay(100);
  }
  ledBlinking = false;
}

void fadeLEDs() {
  // Fade effect doesn't really work - fuck it
  // Just Turning Them On
  FastLED.setBrightness(255);
  FastLED.show();


  // Reset brightness to 0 before starting the fade up effect
  // FastLED.setBrightness(0);

  // for (int i = 0; i <= 100; i++) {
  //   FastLED.setBrightness(map(i, 0, 100, 0, 255));
  //   FastLED.show();
  //   delay(5);
  // }
}

void fadeOutLEDs() {
  for (int i = 100; i >= 0; i--) {
    FastLED.setBrightness(map(i, 0, 100, 0, 255));
    FastLED.show();
    delay(20);
  }
}


// Mode 1: Gentle fading blue waves - dark blue waves fade from left to right over lighter medium blue and back
void mode1() {
  if (!motionDetection || (motionDetection && motionDetected)) {
    static unsigned long lastUpdate = 0;
    static int direction = 1;
    static int position = 0;

    // Set the base color to lighter medium blue
    fill_solid(leds, NUM_LEDS, CRGB(0, 0, 95));

    // Create the dark blue wave
    for (int i = position - 2; i <= position + 2; i++) {
      if (i >= 0 && i < NUM_LEDS) {
        float factor = 1.0 - abs(position - i) / 2.0;
        CRGB darkBlue = CRGB(0, 0, 255);
        leds[i] = leds[i].lerp8(darkBlue, 255 * factor);
      }
    }

    // Show the updated LEDs
    FastLED.show();

    // Update the position based on the elapsed time
    if (millis() - lastUpdate >= (750 / NUM_LEDS)) {
      position += direction;
      if (position == NUM_LEDS - 1 || position == 0) {
        direction = -direction;
      }
      lastUpdate = millis();
    }
  } else {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
  }
}



// Mode 2: Slowly fade between ROYGBIV colors over 3 minutes
void mode2() {
  if (!motionDetection || (motionDetection && motionDetected)) {
      static uint8_t hue = 0;
      static unsigned long lastColorChangeTime = 0;

      if (millis() - lastColorChangeTime >= 180000) {
        hue += 32;
        lastColorChangeTime = millis();
      }
      uint8_t currentColorIndex = hue / 32;
      CRGB currentColor = CHSV(currentColorIndex * 32, 255, 255);
      CRGB nextColor = CHSV((currentColorIndex + 1) * 32, 255, 255);
      float fadeProgress = (millis() - lastColorChangeTime) / 180000.0;
      CRGB color = currentColor.lerp16(nextColor, (uint16_t)(fadeProgress * 65535));

      fill_solid(leds, NUM_LEDS, color);
      FastLED.show();
  } else {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
  }
}



// Mode 3: LEDs change individually and randomly fading between more saturated colors
void mode3() {
   if (!motionDetection || (motionDetection && motionDetected)) {
    for (int i = 0; i < NUM_LEDS; i++) {
      if (random(0, 100) < 5) {
        static int hue[NUM_LEDS] = {160, 100, 130, 165, 180}; // Define the starting hue values for each LED
        static int targetHue[NUM_LEDS] = {200, 70, 190, 150, 220}; // Define the target hue values for each LED
        static int hueStep[NUM_LEDS] = {2, -3, 4, -5, 3}; // Define the hue step for each LED with faster speed
        static int currentHue[NUM_LEDS] = {0, 0, 0, 0, 0}; // Initialize the current hue values for each LED to 0

        for (int i = 0; i < NUM_LEDS; i++) {
          currentHue[i] += hueStep[i];
          if (currentHue[i] < 0) {
            currentHue[i] = 255 + currentHue[i];
          }
          if (currentHue[i] > 255) {
            currentHue[i] = currentHue[i] - 255;
          }
          if (currentHue[i] == targetHue[i]) {
            targetHue[i] = hue[i];
            hue[i] = currentHue[i];
            hueStep[i] = -hueStep[i];
          }
          leds[i] = CHSV(currentHue[i], 255, 255); // Use full saturation for brighter colors
        }
      }
    }
    FastLED.show();
    FastLED.delay(20);
  } else {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
  }
}

// Mode 4: LEDs quickly sparkle various shades of dark purple and dark magenta
// void mode4() {
//   if (!motionDetection || (motionDetection && motionDetected)) {
//     for (int i = 0; i < NUM_LEDS; i++) {
//       if (random(0, 100) < 10) {
//         leds[i] = CHSV(random(270, 300), random(60, 180), random(50, 100));
//       } else {
//         leds[i] = CRGB::Black;
//       }
//     }
//     FastLED.show();
//     FastLED.delay(50);
//   } else {
//     fill_solid(leds, NUM_LEDS, CRGB::Black);
//     FastLED.show();
//   }
// }


// Mode 4: LEDs sparkle with warm white, amber, or blue colors and move quickly in a random direction when they appear. 
//Sparkles move at a rate of 25ms per LED and disappear when they go out of bounds. The other LEDs use warm white, amber, or blue colors. 
//Sparkles are generated randomly, and their position and direction are also random. 
void mode4() {
  static unsigned long lastUpdate = 0;
  static int sparklePosition = -1;
  static int sparkleDirection = 0;

  if (!motionDetection || (motionDetection && motionDetected)) {
    for (int i = 0; i < NUM_LEDS; i++) {
      if (i == sparklePosition) {
        // Move the sparkle in the current direction
        sparklePosition += sparkleDirection;
        if (sparklePosition < 0 || sparklePosition >= NUM_LEDS) {
          // If the sparkle goes out of bounds, reset it
          sparklePosition = -1;
        }
        leds[i] = CHSV(random(32, 50), random(200, 255), random(150, 200)); // Warm white, amber, or blue
      } else if (random(0, 350) < 10) { // Change the random range to 500 to increase the probability of generating a sparkle
        // Generate a sparkle in a random position and direction
        sparklePosition = random(0, NUM_LEDS);
        sparkleDirection = random(2) * 2 - 1; // -1 or 1
        leds[i] = CHSV(random(32, 50), random(200, 255), random(150, 200)); // Warm white, amber, or blue
      } else {
        leds[i] = CRGB::Black;
      }
    }
    FastLED.show();
    FastLED.delay(25);
  } else {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
  }
}



// Mode 5: Chasing lights with random colors from left to right, looping, and never-ending
void mode5() {
  static unsigned long lastUpdate = 0;
  static int position = 0;

  if (millis() - lastUpdate >= 100) {
    // Shift all LEDs to the right
    for (int i = NUM_LEDS - 1; i > 0; i--) {
      leds[i] = leds[i - 1];
    }

    // Set the first LED to a random color
    leds[0] = CHSV(random(256), 255, 255);

    // Update the position
    position++;
    if (position >= NUM_LEDS) {
      position = 0;
    }

    // Show the updated LEDs
    FastLED.show();

    // Update the last update timestamp
    lastUpdate = millis();
  }
}


