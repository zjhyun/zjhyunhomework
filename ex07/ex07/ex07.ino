#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "iQOO 13";
const char* password = "31415926Btx";
const int LED_PIN = 2;

WebServer server(80);

const int PWM_FREQ = 5000;      // PWM 频率
const int PWM_RESOLUTION = 8;   // 8位分辨率（0-255）

String makePage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>作业7：无极调光器</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 50px; }
    input[type="range"] { width: 300px; height: 25px; }
    h1 { color: #333; }
  </style>
</head>
<body>
  <h1> 无极调光器</h1>
  <p>拖动滑块调节 LED 亮度：</p>
  <input type="range" min="0" max="255" value="0" 
         oninput="updateBrightness(this.value)">
  <p>亮度值：<span id="val">0</span></p>

  <script>
    function updateBrightness(val) {
      document.getElementById('val').innerText = val;
      fetch('/pwm?val=' + val);
    }
  </script>
</body>
</html>
)rawliteral";
  return html;
}

void handleRoot() {
  server.send(200, "text/html; charset=UTF-8", makePage());
}

void handlePwm() {
  if (server.hasArg("val")) {
    int brightness = server.arg("val").toInt();
    brightness = constrain(brightness, 0, 255);
    ledcWrite(LED_PIN, brightness);  
    server.send(200, "text/plain", "OK");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  

  ledcAttach(LED_PIN, PWM_FREQ, PWM_RESOLUTION);  // 引脚、频率、分辨率
  ledcWrite(LED_PIN, 0);                          // 初始关闭
  
  // 连接 WiFi
  WiFi.begin(ssid, password);
  Serial.print("连接 WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi 已连接");
  Serial.print("访问地址: http://");
  Serial.println(WiFi.localIP());
  
  server.on("/", handleRoot);
  server.on("/pwm", handlePwm);
  server.begin();
  Serial.println("服务器已启动");
}

void loop() {
  server.handleClient();
}