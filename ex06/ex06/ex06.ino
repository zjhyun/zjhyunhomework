const int LED_PIN_A = 4;  // LED1引脚
const int LED_PIN_B = 16;  // LED2引脚
const int freq = 5000;    // PWM频率5000Hz
const int resolution = 8; // 8位分辨率（0-255）

// 全局变量
int brightnessA = 0;       // LED1亮度（0-255）
int brightnessB = 0;     // LED2亮度（初始与A反相）
int fadeAmount = 5;        // 亮度变化步长（±5）

void setup() {
  Serial.begin(115200);                  // 串口初始化
  
  // 双LED PWM绑定
  ledcAttach(LED_PIN_A, freq, resolution); // LED1绑定PWM
  ledcAttach(LED_PIN_B, freq, resolution); // LED2绑定PWM
}

void loop() {
  // 1. 双LED反相渐变
  brightnessA += fadeAmount;  // LED1亮度递增
  brightnessB += fadeAmount;  // LED2亮度递减（与A反相）

  // 2. 边界检测：到达0或255时反转方向
  if (brightnessA <= 0 || brightnessA >= 255) {
    fadeAmount = -fadeAmount;  // 反转变化方向（增→减或减→增）
    // 防止亮度越界（可选）
    brightnessA = constrain(brightnessA, 0, 255);
    brightnessB = constrain(brightnessB, 0, 255);
  }

  // 3. 输出PWM亮度
  ledcWrite(LED_PIN_A, brightnessA); // LED1亮度
  ledcWrite(LED_PIN_B, brightnessB); // LED2亮度
  
  delay(30); // 控制双闪速度
}