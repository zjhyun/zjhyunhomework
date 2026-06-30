#define TOUCH_PIN 4       // 触摸传感器引脚
#define LED_PIN 2         // LED引脚
#define THRESHOLD 500     // 触摸阈值
#define DEBOUNCE_TIME 300 // 触摸防抖时间

// 全局变量
volatile bool touchTriggered = false;  // 触摸触发标记（中断修改）
int speedLevel = 1;                     // 当前档位（1-3）
const int delayLevels[3] = {30, 20, 10}; // 各档位延迟（ms）：1档30，2档20，3档10

// 中断函数
void IRAM_ATTR gotTouch() {
  static unsigned long last = 0;
  if (millis() - last > DEBOUNCE_TIME) { // 防抖300ms
    touchTriggered = true;               // 标记触摸触发
    last = millis();
  }
}

void setup() {
  Serial.begin(115200);                  // 串口初始化
  pinMode(LED_PIN, OUTPUT);              // LED引脚设为输出
  digitalWrite(LED_PIN, LOW);            // 初始灭

  // 呼吸灯PWM设置
  const int freq = 5000;                 // 频率5000Hz
  const int resolution = 8;              // 8位分辨率（0-255）
  ledcAttach(LED_PIN, freq, resolution); // 绑定引脚与PWM

  // 触摸中断绑定
  touchAttachInterrupt(TOUCH_PIN, gotTouch, THRESHOLD);
}

void loop() {
  // 1. 触摸检测：切换档位
  if (touchTriggered) {
    touchTriggered = false;              // 清除标记
    speedLevel = (speedLevel % 3) + 1;   // 档位循环：1→2→3→1
    Serial.print("Speed Level: ");       // 串口打印档位（调试用）
    Serial.println(speedLevel);
  }

  // 2. 呼吸灯逻辑
  int delayTime = delayLevels[speedLevel - 1]; // 当前档位延迟
  
  // 逐渐变亮（0→255）
  for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle++) {
    ledcWrite(LED_PIN, dutyCycle);        // 写PWM亮度
    delay(delayTime);                    // 按档位调速
  }
  
  // 逐渐变暗（255→0）
  for (int dutyCycle = 255; dutyCycle >= 0; dutyCycle--) {
    ledcWrite(LED_PIN, dutyCycle);        // 写PWM亮度
    delay(delayTime);                    // 按档位调速
  }

  Serial.println("Breathing cycle completed"); // 串口提示
}