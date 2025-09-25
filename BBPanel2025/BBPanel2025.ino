#include <FastLED.h>
#include "PharaohV2L.h" // Include the generated image header

#define PANEL_SIDE 'L' // Right Panel

// -- Hardware Configuration
#define LED_BUILTIN 2 // On-board LED pin for status indication
#define DATA_PIN_1 4
#define DATA_PIN_2 18
#define DATA_PIN_3 19

#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define BRIGHTNESS 200 // Set brightness (0-255)

// -- Display & Panel Constants (mirrored from your Go code)

// Dimensions of a single LED matrix panel.
const uint16_t PANEL_WIDTH = 16;
const uint16_t PANEL_HEIGHT = 16;
const uint16_t LEDS_PER_PANEL = PANEL_WIDTH * PANEL_HEIGHT;
// Display configuration (number of panels)
const uint16_t DISPLAY_WIDTH = 4;
const uint16_t DISPLAY_HEIGHT = 12;
const uint16_t TOTAL_PANELS = DISPLAY_WIDTH * DISPLAY_HEIGHT; // 48 panels total

// Calculated total dimensions and LED counts
const uint16_t LEDS_PER_ROW = PANEL_WIDTH * DISPLAY_WIDTH;      // Total LEDs in a single horizontal row of the entire display
const uint16_t LEDS_PER_COLUMN = PANEL_HEIGHT * DISPLAY_HEIGHT; // Total LEDs in a single vertical column of the entire display
const uint32_t TOTAL_LEDS = LEDS_PER_ROW * LEDS_PER_COLUMN;     // Total LEDs in the entire display

// -- Multi-pin Setup
// Assuming the total panels are split evenly across the 3 pins.
const uint16_t PANELS_PER_SEGMENT = TOTAL_PANELS / 3;                  // 16 panels per pin
const uint32_t LEDS_PER_SEGMENT = PANELS_PER_SEGMENT * LEDS_PER_PANEL; // 4096 LEDs per pin

// Create a single, contiguous array for all LEDs.
// FastLED will handle mapping sections of this array to the physical output pins.
CRGB leds[TOTAL_LEDS];

// -- Mapping Tables (identical to your Go code)

// Describes the serpentine ("Z") wiring layout within a single 16x16 panel.
const uint8_t PANEL_MAPPING[PANEL_HEIGHT][PANEL_WIDTH] = {
    {0, 31, 32, 63, 64, 95, 96, 127, 128, 159, 160, 191, 192, 223, 224, 255},
    {1, 30, 33, 62, 65, 94, 97, 126, 129, 158, 161, 190, 193, 222, 225, 254},
    {2, 29, 34, 61, 66, 93, 98, 125, 130, 157, 162, 189, 194, 221, 226, 253},
    {3, 28, 35, 60, 67, 92, 99, 124, 131, 156, 163, 188, 195, 220, 227, 252},
    {4, 27, 36, 59, 68, 91, 100, 123, 132, 155, 164, 187, 196, 219, 228, 251},
    {5, 26, 37, 58, 69, 90, 101, 122, 133, 154, 165, 186, 197, 218, 229, 250},
    {6, 25, 38, 57, 70, 89, 102, 121, 134, 153, 166, 185, 198, 217, 230, 249},
    {7, 24, 39, 56, 71, 88, 103, 120, 135, 152, 167, 184, 199, 216, 231, 248},
    {8, 23, 40, 55, 72, 87, 104, 119, 136, 151, 168, 183, 200, 215, 232, 247},
    {9, 22, 41, 54, 73, 86, 105, 118, 137, 150, 169, 182, 201, 214, 233, 246},
    {10, 21, 42, 53, 74, 85, 106, 117, 138, 149, 170, 181, 202, 213, 234, 245},
    {11, 20, 43, 52, 75, 84, 107, 116, 139, 148, 171, 180, 203, 212, 235, 244},
    {12, 19, 44, 51, 76, 83, 108, 115, 140, 147, 172, 179, 204, 211, 236, 243},
    {13, 18, 45, 50, 77, 82, 109, 114, 141, 146, 173, 178, 205, 210, 237, 242},
    {14, 17, 46, 49, 78, 81, 110, 113, 142, 145, 174, 177, 206, 209, 238, 241},
    {15, 16, 47, 48, 79, 80, 111, 112, 143, 144, 175, 176, 207, 208, 239, 240},
};

// Describes the physical arrangement of the panels on the display.
const uint8_t SCREEN_MAPPING[DISPLAY_HEIGHT][DISPLAY_WIDTH] = {
    {0, 1, 2, 3}, // Panels 0-15 are on Segment 1
    {4, 5, 6, 7},
    {8, 9, 10, 11},
    {12, 13, 14, 15},
    {16, 17, 18, 19}, // Panels 16-31 are on Segment 2
    {20, 21, 22, 23},
    {24, 25, 26, 27},
    {28, 29, 30, 31},
    {32, 33, 34, 35}, // Panels 32-47 are on Segment 3
    {36, 37, 38, 39},
    {40, 41, 42, 43},
    {44, 45, 46, 47},
};

/**
 * @brief Calculates the global linear index for a pixel based on its (x, y) coordinates.
 * This function's logic is identical to the original Go program.
 */
uint32_t FindPixelIndex(uint16_t x, uint16_t y)
{
  if (x >= LEDS_PER_ROW || y >= LEDS_PER_COLUMN)
  {
    return TOTAL_LEDS; // Return an invalid index
  }
  uint16_t panelX = x / PANEL_WIDTH;
  uint16_t panelY = y / PANEL_HEIGHT;
  uint8_t panelIndex = SCREEN_MAPPING[panelY][panelX];

  uint16_t xInPanel = x % PANEL_WIDTH;
  uint16_t yInPanel = y % PANEL_HEIGHT;
  uint16_t pixelInPanelIndex = PANEL_MAPPING[yInPanel][xInPanel];

  uint32_t finalIndex = (uint32_t(panelIndex) * LEDS_PER_PANEL) + pixelInPanelIndex;

  return finalIndex;
}

/**
 * @brief Sets the color of a pixel at a given (x, y) coordinate.
 * It calculates the global index and writes the color to the single 'leds' buffer.
 */
void drawPixel(uint16_t x, uint16_t y, CRGB color)
{
  uint32_t globalIndex = FindPixelIndex(x, y);
  if (globalIndex < TOTAL_LEDS)
  {
    leds[globalIndex] = color;
  }
}

// ---- NEW FUNCTION ----
/**
 * @brief Renders an image stored in PROGMEM onto the LED matrix.
 * @param img A reference to the ImageData struct, which is defined in PROGMEM.
 */
void drawImage(const ImageData &img)
{
  // Read the number of colors and the pointer to the color data array from PROGMEM
  uint8_t color_count = pgm_read_byte(&img.color_count);
  const ColorData *color_data_array_ptr = (const ColorData *)pgm_read_ptr(&img.data);

  char name_buffer[32];
  strcpy_P(name_buffer, img.name);
  Serial.print("Drawing image '");
  Serial.print(name_buffer);
  Serial.print("' with ");
  Serial.print(color_count);
  Serial.println(" colors.");

  // Iterate through each color group in the image data
  for (uint8_t i = 0; i < color_count; i++)
  {
    // Create a temporary struct in RAM to hold the ColorData read from PROGMEM
    ColorData currentColorData;
    memcpy_P(&currentColorData, &color_data_array_ptr[i], sizeof(ColorData));

    // Extract the color, point count, and the pointer to the points array
    CRGB pixelColor = CRGB(currentColorData.color);
    uint16_t pointCount = currentColorData.point_count;
    const ColorPoint *points_ptr = currentColorData.points;

    // Iterate through each point for the current color
    for (uint16_t j = 0; j < pointCount; j++)
    {
      // Create a temporary struct for the current point
      ColorPoint currentPoint;
      memcpy_P(&currentPoint, &points_ptr[j], sizeof(ColorPoint));

      // Use the existing drawPixel function to set the LED in the buffer
      drawPixel(currentPoint.x, currentPoint.y, pixelColor);
    }
  }
  Serial.println("Image data loaded into buffer.");
}

// -- Arduino Setup & Loop --

void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("\n\n--- Starting LED Matrix Controller (3-Pin, Single Buffer) ---");
  Serial.println("--- Configuration ---");
  Serial.print("Display Dimensions (pixels): ");
  Serial.print(LEDS_PER_ROW);
  Serial.print(" x ");
  Serial.println(LEDS_PER_COLUMN);
  Serial.print("Total LEDs: ");
  Serial.println(TOTAL_LEDS);
  Serial.print("Total Panels: ");
  Serial.println(TOTAL_PANELS);
  Serial.print("LEDs per Panel: ");
  Serial.println(LEDS_PER_PANEL);
  Serial.println("--- Segmentation ---");
  Serial.print("Segments (Pins): 3");
  Serial.print(" | Panels per Segment: ");
  Serial.println(PANELS_PER_SEGMENT);
  Serial.print("LEDs per Segment: ");
  Serial.println(LEDS_PER_SEGMENT);
  Serial.println("--------------------");

  // Initialize FastLED for all three segments, pointing to different sections of the same 'leds' array.
  Serial.println("Initializing FastLED controllers...");
  // Segment 1: leds[0] to leds[4095]
  FastLED.addLeds<LED_TYPE, DATA_PIN_1, COLOR_ORDER>(leds, LEDS_PER_SEGMENT).setCorrection(TypicalLEDStrip);
  // Segment 2: leds[4096] to leds[8191]
  FastLED.addLeds<LED_TYPE, DATA_PIN_2, COLOR_ORDER>(leds + LEDS_PER_SEGMENT, LEDS_PER_SEGMENT).setCorrection(TypicalLEDStrip);
  // Segment 3: leds[8192] to leds[12287]
  FastLED.addLeds<LED_TYPE, DATA_PIN_3, COLOR_ORDER>(leds + (2 * LEDS_PER_SEGMENT), LEDS_PER_SEGMENT).setCorrection(TypicalLEDStrip);

  FastLED.setBrightness(BRIGHTNESS);
  Serial.print("Brightness set to: ");
  Serial.println(BRIGHTNESS);

  // Clear all LEDs on startup
  Serial.println("Clearing all LEDs...");
  FastLED.clear();
  FastLED.show();
  delay(500);

  // --- MODIFIED SECTION ---
  Serial.println("Setup complete. Starting to draw image...");
  FastLED.clear(); // Ensure the buffer is empty before drawing

  // Call the new function to draw the image from Amun.h
  drawImage(IMAGE_DATA);

  // Push the data from the buffer to the LEDs
  FastLED.show();
  Serial.println("Image display command sent.");
}

// Define the time unit for Morse Code in milliseconds
int unit_time = 250;

// Function to generate a "dit" (short pulse)
void dit()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(unit_time); // LED is on for 1 unit
  digitalWrite(LED_BUILTIN, LOW);
  delay(unit_time); // Space between elements is 1 unit
}

// Function to generate a "dah" (long pulse)
void dah()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(3 * unit_time); // LED is on for 3 units
  digitalWrite(LED_BUILTIN, LOW);
  delay(unit_time); // Space between elements is 1 unit
}

// The main loop repeats forever
void loop()
{
#if PANEL_SIDE == 'L'
  // Morse code for "L" is: dit-dah-dit-dit (· — · ·)
  dit();
  dah();
  dit();
  dit();
#elif PANEL_SIDE == 'R'
  // Morse code for "R" is: dit-dah-dit (· — ·)
  dit();
  dah();
  dit();
#else
#error "PANEL_SIDE must be defined as L or R"
#endif

  // Wait for a longer period before repeating the letter.
  // The standard space between letters is 3 units. Since our helper functions
  // already add a 1-unit space, we add an extra 2 units here.
  delay(2 * unit_time);
}