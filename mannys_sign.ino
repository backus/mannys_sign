#include <FastLED.h>

FASTLED_USING_NAMESPACE

#include <time.h>
#include <sys/time.h>


// FastLED "100-lines-of-code" demo reel, showing just a few
// of the kinds of animation patterns you can quickly and easily
// compose using FastLED.
//
// This example also shows one easy way to define multiple
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN__S2 19 // 
#define DATA_PIN__S3 21 // GPIO21
#define DATA_PIN__S4 2  // S4, hours digits, GPIO2 / ADC12 / TOUCH2
#define DATA_PIN__S5 22 // S5, hours label, GPIO22
#define DATA_PIN__S6 4  // S6, mins digits, GPIO4 / ADC12 / TOUCH0
#define DATA_PIN__S9 17 // GPIO17
#define DATA_PIN__S7 0 // S7, mins label, GPIO18 / VSPI SCK
#define DATA_PIN__S8 16
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

#define NUM_LEDS__S2 200
#define NUM_LEDS__S4 200
#define NUM_LEDS__S6 200
#define NUM_LEDS__S9 5
#define NUM_LEDS__S7 50
#define NUM_LEDS__S5 50
#define NUM_LEDS__S3 50
#define NUM_LEDS__S8 200

CRGB ledsDays[NUM_LEDS__S2];
CRGB ledsHours[NUM_LEDS__S4];
CRGB ledsMinutes[NUM_LEDS__S6];
CRGB ledsSeconds[NUM_LEDS__S8];
CRGB ledsMinsLabel[NUM_LEDS__S7];
CRGB ledsHoursLabel[NUM_LEDS__S5];
CRGB ledsDaysLabel[NUM_LEDS__S3];
CRGB ledsSecsLabel[NUM_LEDS__S9];

#define BRIGHTNESS         64
#define FRAMES_PER_SECOND  60

/* ESP32 specific stuff to avoid flickering?? */

// -- The core to run FastLED.show()
#define FASTLED_SHOW_CORE 0

// -- Task handles for use in the notifications
static TaskHandle_t FastLEDshowTaskHandle = 0;
static TaskHandle_t userTaskHandle = 0;

/** show() for ESP32
    Call this function instead of FastLED.show(). It signals core 0 to issue a show,
    then waits for a notification that it is done.
*/
void FastLEDshowESP32()
{
  if (userTaskHandle == 0) {
    // -- Store the handle of the current task, so that the show task can
    //    notify it when it's done
    userTaskHandle = xTaskGetCurrentTaskHandle();

    // -- Trigger the show task
    xTaskNotifyGive(FastLEDshowTaskHandle);

    // -- Wait to be notified that it's done
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 200 );
    ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
    userTaskHandle = 0;
  }
}

/** show Task
    This function runs on core 0 and just waits for requests to call FastLED.show()
*/
void FastLEDshowTask(void *pvParameters)
{
  // -- Run forever...
  for (;;) {
    // -- Wait for the trigger
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // -- Do the show (synchronously)
    FastLED.show();

    // -- Notify the calling task
    xTaskNotifyGive(userTaskHandle);
  }
}

struct DigitRegion {
  uint8_t start;
  uint8_t end;
};

struct DigitLayout {
  struct DigitRegion top;
  struct DigitRegion leftTop;
  struct DigitRegion leftBottom;
  struct DigitRegion rightTop;
  struct DigitRegion rightBottom;
  struct DigitRegion bottom;
  struct DigitRegion center;
};

struct DigitLayout daysDigit1 = {
  { 0, 8 },
  { 48, 58 }, // left top
  { 40, 49 }, // left bottom
  { 10, 19 }, // right top
  { 18, 29 }, // right bottom
  { 28, 39 },
  { 68, 78 }
};

struct DigitLayout daysDigit2 = {
  { 91, 105 },
  { 106, 116 }, // left top
  { 115, 124 }, // left bottom
  { 151, 160 }, // right top
  { 139, 152 }, // right bottom
  { 125, 138 },
  { 80, 89 }
};

struct DigitLayout hoursDigit1 = {
  { 0, 8 },
  { 48, 58 }, // left top
  { 40, 49 }, // left bottom
  { 10, 19 }, // right top
  { 18, 29 }, // right bottom
  { 28, 39 },
  { 68, 78 }
};

struct DigitLayout hoursDigit2 = {
  { 91, 105 },
  { 106, 116 }, // left top
  { 115, 124 }, // left bottom
  { 151, 160 }, // right top
  { 139, 152 }, // right bottom
  { 125, 138 },
  { 80, 89 }
};

struct DigitLayout minsDigit1 = {
  { 0, 8 },
  { 48, 58 }, // left top
  { 40, 49 }, // left bottom
  { 10, 19 }, // right top
  { 18, 29 }, // right bottom
  { 28, 39 },
  { 68, 78 }
};

struct DigitLayout minsDigit2 = {
  { 91, 105 },
  { 106, 116 }, // left top
  { 115, 124 }, // left bottom
  { 151, 160 }, // right top
  { 139, 152 }, // right bottom
  { 125, 138 },
  { 80, 89 }
};

struct DigitLayout secsDigit1 = {
  { 0, 8 },
  { 48, 58 }, // left top
  { 40, 49 }, // left bottom
  { 10, 19 }, // right top
  { 18, 29 }, // right bottom
  { 28, 39 },
  { 68, 78 }
};

struct DigitLayout secsDigit2 = {
  { 91, 105 },
  { 106, 116 }, // left top
  { 115, 124 }, // left bottom
  { 151, 160 }, // right top
  { 139, 152 }, // right bottom
  { 125, 138 },
  { 80, 89 }
};

struct DigitDisplay {
  struct DigitLayout layout;
  CRGB *leds;
};

const struct DigitDisplay days1 = {
  daysDigit1,
  ledsDays
};

const struct DigitDisplay days2 = {
  daysDigit2,
  ledsDays
};

const struct DigitDisplay hours1 = {
  hoursDigit1,
  ledsHours
};

const struct DigitDisplay hours2 = {
  hoursDigit2,
  ledsHours
};

const struct DigitDisplay mins1 = {
  minsDigit1,
  ledsMinutes
};

const struct DigitDisplay mins2 = {
  minsDigit2,
  ledsMinutes
};

const struct DigitDisplay secs1 = {
  secsDigit1,
  ledsSeconds
};

const struct DigitDisplay secs2 = {
  secsDigit2,
  ledsSeconds
};

//#include "RTCZero.h"
//
//RTCZero rtc;

void setupRtc() {
  //  rtc.begin();
  byte seconds, minutes, hours;
  byte days, months, years;
  int thour, tminute, tsecond;
  int tmonth, tday, tyear;
  char s_month[5];
  static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
  sscanf(__DATE__, "%s %d %d", s_month, &tday, &tyear);
  sscanf(__TIME__, "%d:%d:%d", &thour, &tminute, &tsecond);
  tmonth = (strstr(month_names, s_month) - month_names) / 3;
  years = tyear - 2000;
  months =  tmonth + 1;
  days = tday;
  hours = thour;
  minutes = tminute;
  seconds = tsecond;

  Serial.print("tmonth: ");
  Serial.println(tmonth);
  Serial.print("years: ");
  Serial.println(years);
  Serial.print("months: ");
  Serial.println(months);
  Serial.print("days: ");
  Serial.println(days);
  Serial.print("hours: ");
  Serial.println(hours);
  Serial.print("minutes: ");
  Serial.println(minutes);
  Serial.print("seconds: ");
  Serial.println(seconds);
  Serial.println("---");

  Serial.print("before ");
  time_t now;

  time(&now);
  Serial.println(now);

  //  setTime(hours,minutes,seconds,days,months,tyear);

  Serial.print("after ");
  time(&now);
  Serial.println(now);
  //  rtc.setTime(hours, minutes, seconds);
  //  rtc.setDate(days, months, years);
}

void setup() {
  delay(3000); // 3 second delay for recovery

  Serial.begin(9600);

  setupRtc();

  pinMode(DATA_PIN__S2, OUTPUT);
  pinMode(DATA_PIN__S3, OUTPUT);
  pinMode(DATA_PIN__S4, OUTPUT);
  pinMode(DATA_PIN__S5, OUTPUT);
  pinMode(DATA_PIN__S6, OUTPUT);
  pinMode(DATA_PIN__S9, OUTPUT);
  pinMode(DATA_PIN__S7, OUTPUT);
  pinMode(DATA_PIN__S8, OUTPUT);

  FastLED.addLeds<LED_TYPE, DATA_PIN__S2, COLOR_ORDER>(ledsDays, NUM_LEDS__S2).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, DATA_PIN__S4, COLOR_ORDER>(ledsHours, NUM_LEDS__S4).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, DATA_PIN__S6, COLOR_ORDER>(ledsMinutes, NUM_LEDS__S6).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, DATA_PIN__S8, COLOR_ORDER>(ledsSeconds, NUM_LEDS__S8).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, DATA_PIN__S7, COLOR_ORDER>(ledsSecsLabel, NUM_LEDS__S7).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, DATA_PIN__S5, COLOR_ORDER>(ledsHoursLabel, NUM_LEDS__S5).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, DATA_PIN__S9, COLOR_ORDER>(ledsMinsLabel, NUM_LEDS__S9).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, DATA_PIN__S3, COLOR_ORDER>(ledsDaysLabel, NUM_LEDS__S3).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  int core = xPortGetCoreID();
  Serial.print("Main code running on core ");
  Serial.println(core);

  Serial.print("Compiled at ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);

  // -- Create the FastLED show task
  xTaskCreatePinnedToCore(FastLEDshowTask, "FastLEDshowTask", 2048, NULL, 2, &FastLEDshowTaskHandle, FASTLED_SHOW_CORE);
}

void fill_region(struct DigitRegion &region, const CRGB &color) {
  fill_solid(ledsHours + region.start, region.end - region.start, color);
}

void paint_region(struct CRGB *leds, const struct DigitRegion &region) {
  fill_solid(leds + region.start, region.end - region.start, CRGB::Red);
}

void paintRight(const struct DigitDisplay &disp) {
  paint_region(disp.leds, disp.layout.rightTop);
  paint_region(disp.leds, disp.layout.rightBottom);
}

void paintLeft(const struct DigitDisplay &disp) {
  paint_region(disp.leds, disp.layout.leftTop);
  paint_region(disp.leds, disp.layout.leftBottom);
}

void draw1(const struct DigitDisplay &disp) {
  paintRight(disp);
}

void draw2(const struct DigitDisplay &disp) {
  paint_region(disp.leds, disp.layout.top);
  paint_region(disp.leds, disp.layout.center);
  paint_region(disp.leds, disp.layout.bottom);

  paint_region(disp.leds, disp.layout.rightTop);
  paint_region(disp.leds, disp.layout.leftBottom);
}

void draw3(const struct DigitDisplay &disp) {
  paint_region(disp.leds, disp.layout.top);
  paint_region(disp.leds, disp.layout.center);
  paint_region(disp.leds, disp.layout.bottom);
  paintRight(disp);
}

void draw4(const struct DigitDisplay &disp) {
  paint_region(disp.leds, disp.layout.center);
  paint_region(disp.leds, disp.layout.leftTop);
  paintRight(disp);
}

void draw5(const struct DigitDisplay &disp) {
  paint_region(disp.leds, disp.layout.top);
  paint_region(disp.leds, disp.layout.center);
  paint_region(disp.leds, disp.layout.bottom);
  paint_region(disp.leds, disp.layout.leftTop);
  paint_region(disp.leds, disp.layout.rightBottom);
}

void draw6(const struct DigitDisplay &disp) {
  paint_region(disp.leds, disp.layout.top);
  paint_region(disp.leds, disp.layout.center);
  paint_region(disp.leds, disp.layout.bottom);
  paint_region(disp.leds, disp.layout.rightBottom);
  paintLeft(disp);
}

void draw7(const struct DigitDisplay &disp) {
  paint_region(disp.leds, disp.layout.top);
  paintRight(disp);
}

void draw8(const struct DigitDisplay &disp) {
  paintRight(disp);
  paintLeft(disp);

  paint_region(disp.leds, disp.layout.top);
  paint_region(disp.leds, disp.layout.center);
  paint_region(disp.leds, disp.layout.bottom);
}

void draw9(const struct DigitDisplay &disp) {
  paintRight(disp);

  paint_region(disp.leds, disp.layout.leftTop);
  paint_region(disp.leds, disp.layout.top);
  paint_region(disp.leds, disp.layout.center);
}

void draw0(const struct DigitDisplay &disp) {
  paintRight(disp);
  paintLeft(disp);
  paint_region(disp.leds, disp.layout.top);
  paint_region(disp.leds, disp.layout.bottom);
}

void drawDigit(const struct DigitDisplay &disp, time_t digit) {
  switch (digit) {
    case 0:
      draw0(disp);
      break;
    case 1:
      draw1(disp);
      break;
    case 2:
      draw2(disp);
      break;
    case 3:
      draw3(disp);
      break;
    case 4:
      draw4(disp);
      break;
    case 5:
      draw5(disp);
      break;
    case 6:
      draw6(disp);
      break;
    case 7:
      draw7(disp);
      break;
    case 8:
      draw8(disp);
      break;
    case 9:
      draw9(disp);
      break;
    default:
      Serial.println("WTF? Digit wasn't 0-9");
  }
}

uint8_t gHue = 0;

void loop() {
  FastLED.clear();

  static struct timeval now;
  gettimeofday(&now, NULL);

  time_t seconds = now.tv_sec % 60;

  drawDigit(hours1, seconds / 10);
  drawDigit(hours2, seconds % 10);
  drawDigit(mins1, seconds / 10);
  drawDigit(mins2, seconds % 10);
  drawDigit(secs1, seconds / 10);
  drawDigit(secs2, seconds % 10);
  drawDigit(days1, seconds / 10);
  drawDigit(days2, seconds % 10);

  fill_solid(ledsMinsLabel, NUM_LEDS__S9, CRGB::Green);
  fill_solid(ledsSecsLabel, NUM_LEDS__S7, CRGB::Green);
  fill_solid(ledsHoursLabel, NUM_LEDS__S5, CRGB::Red);
  fill_solid(ledsDaysLabel, NUM_LEDS__S3, CRGB::Red);

  // send the 'leds' array out to the actual LED strip
  FastLEDshowESP32();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);

  Serial.println("loop");

  EVERY_N_MILLISECONDS(20) {
    gHue++;
  }
}
