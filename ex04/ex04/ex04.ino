#define TOUCH_PIN 4
#define LED_PIN 2
#define THRESHOLD 500  

volatile bool touchTriggered = false;
bool ledState = false;

// 中断函数：只打标记
void IRAM_ATTR gotTouch() {
  static unsigned long last = 0;
  if (millis() - last > 300) {  // 防抖 300ms
    touchTriggered = true;
    last = millis();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  touchAttachInterrupt(TOUCH_PIN, gotTouch, THRESHOLD);
}

void loop() {
  if (touchTriggered) {
    touchTriggered = false;
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    Serial.println(ledState ? "ON" : "OFF");
  }
}
