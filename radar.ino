/*
 =====================================================
   RADAR 180° TFT
   Arduino UNO + ST7735 + HC-SR04 + Servo
 =====================================================

   DISPLAY:
   TFT ST7735 1.8" (128x160)
   Orientation: LANDSCAPE

   FEATURES:
   - Smooth radar sweep
   - 180° optimized UI
   - Distance rings
   - Angle markers
   - Object trail
   - HUD information
   - Cleaner graphics

 =====================================================
   TFT ST7735 CONNECTIONS
 =====================================================

   TFT        -> UNO

   VCC        -> 5V
   GND        -> GND
   SCL/CLK    -> D13
   SDA/MOSI   -> D11
   CS         -> D5
   DC         -> D7
   RST        -> D8
   LED        -> 5V

 =====================================================
   HC-SR04
 =====================================================

   VCC        -> 5V
   GND        -> GND
   TRIG       -> D2
   ECHO       -> D3

 =====================================================
   SERVO
 =====================================================

   SIGNAL     -> D6
   VCC        -> 5V
   GND        -> GND

 =====================================================
   LIBRARIES:
   - Adafruit GFX
   - Adafruit ST7735
 =====================================================
*/

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <Servo.h>

// ================= TFT =================
#define TFT_CS 5
#define TFT_RST 8
#define TFT_DC 7

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// ================= ULTRASONIC =================
#define TRIG_PIN 2
#define ECHO_PIN 3

// ================= SERVO =================
#define SERVO_PIN 6

Servo radarServo;

// ================= RADAR =================

// LANDSCAPE:
// Width  = 160
// Height = 128

const int centerX = 80;
const int centerY = 115;

const int radarRadius = 100;

int lastAngle = 0;

// ================= COLORS =================

#define BG_COLOR ST77XX_BLACK
#define GRID_COLOR ST77XX_GREEN
#define SWEEP_COLOR ST77XX_GREEN
#define TARGET_COLOR ST77XX_RED
#define TEXT_COLOR ST77XX_WHITE
#define HUD_COLOR ST77XX_CYAN

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  radarServo.attach(SERVO_PIN);

  tft.initR(INITR_BLACKTAB);

  tft.setRotation(1);

  // velocidade SPI máxima
  SPI.setClockDivider(SPI_CLOCK_DIV2);

  drawRadarUI();
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  // 0 -> 180
  for (int angle = 0; angle <= 180; angle += 2) {

    radarServo.write(angle);

    int dist = getDistance();

    drawSweep(angle, dist);

    delay(8);
  }

  // 180 -> 0
  for (int angle = 180; angle >= 0; angle -= 2) {

    radarServo.write(angle);

    int dist = getDistance();

    drawSweep(angle, dist);

    delay(8);
  }
}

// =====================================================
// DISTANCE
// =====================================================

int getDistance() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  int distance = duration * 0.034 / 2;

  if (distance <= 0 || distance > 100)
    distance = 100;

  return distance;
}

// =====================================================
// DRAW STATIC UI
// =====================================================

void drawRadarUI() {

  tft.fillScreen(BG_COLOR);

  // Distance arcs
  tft.drawCircle(centerX, centerY, 25, GRID_COLOR);
  tft.drawCircle(centerX, centerY, 50, GRID_COLOR);
  tft.drawCircle(centerX, centerY, 75, GRID_COLOR);
  tft.drawCircle(centerX, centerY, 100, GRID_COLOR);

  // Base line
  tft.drawLine(0, centerY, 160, centerY, GRID_COLOR);

  // Angle lines
  drawAngleLine(0);
  drawAngleLine(30);
  drawAngleLine(60);
  drawAngleLine(90);
  drawAngleLine(120);
  drawAngleLine(150);
  drawAngleLine(180);

  // Labels
  tft.setTextSize(1);
  tft.setTextColor(HUD_COLOR);

  tft.setCursor(2, 2);
  tft.print("RADAR 180");

  // Distance labels
  tft.setCursor(130, 102);
  tft.print("25");

  tft.setCursor(130, 77);
  tft.print("50");

  tft.setCursor(130, 52);
  tft.print("75");

  tft.setCursor(124, 27);
  tft.print("100");
}

// =====================================================
// DRAW ANGLE LINES
// =====================================================

void drawAngleLine(int angle) {

  float rad = radians(angle);

  int x = centerX + radarRadius * cos(rad - PI);
  int y = centerY + radarRadius * sin(rad - PI);

  tft.drawLine(centerX, centerY, x, y, GRID_COLOR);
}

// =====================================================
// DRAW SWEEP
// =====================================================

void drawSweep(int angle, int distance) {

  // =========================
  // APAGAR LINHA ANTERIOR
  // =========================

  float lastRad = radians(lastAngle);

  int lastX = centerX + radarRadius * cos(lastRad - PI);
  int lastY = centerY + radarRadius * sin(lastRad - PI);

  // apaga linha antiga
  tft.drawLine(centerX, centerY, lastX, lastY, BG_COLOR);

  // redesenha grades por cima
  drawAngleLine(lastAngle);

  // =========================
  // APAGAR ALVO ANTERIOR
  // =========================

  static int oldObjX = -1;
  static int oldObjY = -1;
  static bool hadTarget = false;

  if (hadTarget) {

    // apaga alvo antigo
    tft.fillCircle(oldObjX, oldObjY, 8, BG_COLOR);

    // redesenha radar na região apagada
    tft.drawCircle(centerX, centerY, 25, GRID_COLOR);
    tft.drawCircle(centerX, centerY, 50, GRID_COLOR);
    tft.drawCircle(centerX, centerY, 75, GRID_COLOR);
    tft.drawCircle(centerX, centerY, 100, GRID_COLOR);

    // redesenha linha base
    tft.drawLine(0, centerY, 160, centerY, GRID_COLOR);

    // redesenha linhas angulares
    drawAngleLine(0);
    drawAngleLine(30);
    drawAngleLine(60);
    drawAngleLine(90);
    drawAngleLine(120);
    drawAngleLine(150);
    drawAngleLine(180);

    hadTarget = false;
  }

  // =========================
  // NOVA LINHA
  // =========================

  float rad = radians(angle);

  int x = centerX + radarRadius * cos(rad - PI);
  int y = centerY + radarRadius * sin(rad - PI);

  // glow
  for (int i = 3; i >= 1; i--) {

    int glowAngle = angle - (i * 2);

    if (glowAngle < 0)
      continue;

    float glowRad = radians(glowAngle);

    int gx = centerX + radarRadius * cos(glowRad - PI);
    int gy = centerY + radarRadius * sin(glowRad - PI);

    tft.drawLine(centerX, centerY, gx, gy, ST77XX_GREEN);
  }

  // linha principal
  tft.drawLine(centerX, centerY, x, y, SWEEP_COLOR);

  // =========================
  // NOVO ALVO
  // =========================

  int objRadius = map(distance, 0, 100, 0, radarRadius);

  int objX = centerX + objRadius * cos(rad - PI);
  int objY = centerY + objRadius * sin(rad - PI);

  if (distance < 100) {

    tft.fillCircle(objX, objY, 4, TARGET_COLOR);
    tft.drawCircle(objX, objY, 7, TARGET_COLOR);

    oldObjX = objX;
    oldObjY = objY;

    hadTarget = true;
  }

  // =========================
  // HUD
  // =========================

  tft.fillRect(0, 0, 70, 30, BG_COLOR);

  tft.drawRect(0, 0, 70, 30, HUD_COLOR);

  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(1);

  tft.setCursor(5, 5);
  tft.print("ANG:");
  tft.print(angle);
  tft.print((char)247);

  tft.setCursor(5, 18);
  tft.print("DST:");
  tft.print(distance);
  tft.print("cm");

  // Distance labels
  tft.setCursor(130, 102);
  tft.print("25");

  tft.setCursor(130, 77);
  tft.print("50");

  tft.setCursor(130, 52);
  tft.print("75");

  tft.setCursor(124, 27);
  tft.print("100");

  // salva ângulo
  lastAngle = angle;
}