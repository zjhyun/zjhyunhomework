#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "iQOO 13";
const char* password = "31415926Btx";

const int LED_PIN = 2;        // 警报 LED（GPIO2）
const int TOUCH_PIN = 4;      // 触摸传感器引脚（GPIO4）

WebServer server(80);

// 状态变量
bool alarmArmed = false;      // 布防状态
bool alarmTriggered = false;  // 报警触发状态
int touchThreshold = 500;     // 触摸阈值

// PWM 配置
const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8;

String makePage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>作业8：物联网安防报警器</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 30px; }
    .container { max-width: 500px; margin: 0 auto; padding: 20px; }
    .status-box { 
      padding: 20px; 
      margin: 20px 0; 
      border-radius: 10px; 
      font-size: 24px;
      font-weight: bold;
    }
    .disarmed { background-color: #4CAF50; color: white; }
    .armed { background-color: #FF9800; color: white; }
    .alarm { background-color: #F44336; color: white; animation: blink 1s infinite; }
    
    @keyframes blink {
      0% { opacity: 1; }
      50% { opacity: 0.5; }
      100% { opacity: 1; }
    }
    
    button {
      padding: 15px 30px;
      margin: 10px;
      font-size: 18px;
      border: none;
      border-radius: 5px;
      cursor: pointer;
      transition: all 0.3s;
    }
    
    .arm-btn { background-color: #2196F3; color: white; }
    .disarm-btn { background-color: #9C27B0; color: white; }
    
    button:hover { transform: scale(1.05); }
    
    .info {
      background-color: #f5f5f5;
      padding: 15px;
      border-radius: 5px;
      margin-top: 20px;
      text-align: left;
    }
    
    .threshold-info {
      background-color: #e3f2fd;
      padding: 10px;
      border-radius: 5px;
      margin-top: 10px;
      font-size: 14px;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1> 物联网安防报警器</h1>
    
    <div id="statusBox" class="status-box disarmed">
      状态：未布防
    </div>
    
    <div>
      <button class="arm-btn" onclick="armSystem()"> 布防</button>
      <button class="disarm-btn" onclick="disarmSystem()"> 撤防</button>
    </div>
    
    <div class="info">
      <h3>使用说明：</h3>
      <ul>
        <li><strong>布防</strong>：开启安防监控，触摸传感器触发报警</li>
        <li><strong>撤防</strong>：关闭安防监控，解除报警</li>
        <li>触摸 GPIO4 引脚可触发报警（布防状态下）</li>
      </ul>
      <div class="threshold-info">
        <strong>当前设置：</strong>触摸阈值 = 500<br>
        触摸值低于 500 时触发报警
      </div>
    </div>
  </div>

  <script>
    // 更新状态显示
    function updateStatus() {
      fetch('/status')
        .then(response => response.json())
        .then(data => {
          const statusBox = document.getElementById('statusBox');
          statusBox.innerHTML = data.message;
          
          // 更新样式类
          statusBox.className = 'status-box';
          if (data.armed && data.triggered) {
            statusBox.classList.add('alarm');
          } else if (data.armed) {
            statusBox.classList.add('armed');
          } else {
            statusBox.classList.add('disarmed');
          }
        });
    }
    
    function armSystem() {
      fetch('/arm')
        .then(response => response.text())
        .then(data => {
          console.log(data);
          updateStatus();
        });
    }
    
    function disarmSystem() {
      fetch('/disarm')
        .then(response => response.text())
        .then(data => {
          console.log(data);
          updateStatus();
        });
    }
    
    // 每2秒更新一次状态
    setInterval(updateStatus, 2000);
    window.onload = updateStatus;
  </script>
</body>
</html>
)rawliteral";
  return html;
}

void handleRoot() {
  server.send(200, "text/html; charset=UTF-8", makePage());
}

void handleArm() {
  alarmArmed = true;
  alarmTriggered = false;
  Serial.println(" 系统已布防");
  server.send(200, "text/plain", "系统已布防");
}

void handleDisarm() {
  alarmArmed = false;
  alarmTriggered = false;
  ledcWrite(LED_PIN, 0);  // 关闭LED
  Serial.println(" 系统已撤防");
  server.send(200, "text/plain", "系统已撤防");
}

void handleStatus() {
  String message = "状态：";
  if (!alarmArmed) {
    message = "状态：未布防";
  } else if (alarmTriggered) {
    message = "状态：报警中！";
  } else {
    message = "状态：已布防（监控中）";
  }
  
  String json = "{";
  json += "\"armed\": " + String(alarmArmed ? "true" : "false") + ",";
  json += "\"triggered\": " + String(alarmTriggered ? "true" : "false") + ",";
  json += "\"message\": \"" + message + "\"";
  json += "}";
  
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // 初始化 LED PWM
  ledcAttach(LED_PIN, PWM_FREQ, PWM_RESOLUTION);
  ledcWrite(LED_PIN, 0);
  
  // 连接 WiFi
  WiFi.begin(ssid, password);
  Serial.print("连接 WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n WiFi 已连接");
  Serial.print(" 访问地址: http://");
  Serial.println(WiFi.localIP());
  
  // 打印触摸阈值信息
  Serial.println("=================================");
  Serial.print("触摸阈值设置为: ");
  Serial.println(touchThreshold);
  Serial.println("请触摸 GPIO4 引脚，观察数值变化");
  Serial.print("当前触摸值: ");
  Serial.println(touchRead(TOUCH_PIN));
  Serial.println("=================================");
  
  // 设置 Web 服务器路由
  server.on("/", handleRoot);
  server.on("/arm", handleArm);
  server.on("/disarm", handleDisarm);
  server.on("/status", handleStatus);
  server.begin();
  Serial.println(" 安防系统已启动");
}

void loop() {
  server.handleClient();
  
  // 安防逻辑
  if (alarmArmed) {
    // 读取触摸值
    int touchValue = touchRead(TOUCH_PIN);
    
    // 每秒打印一次触摸值（便于调试）
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 1000) {
      lastPrint = millis();
      Serial.print("触摸值: ");
      Serial.print(touchValue);
      Serial.print(" | 阈值: ");
      Serial.println(touchThreshold);
    }
    
    // 如果触摸值低于阈值，触发报警
    if (touchValue < touchThreshold) {
      alarmTriggered = true;
    }
    
    // 如果报警被触发
    if (alarmTriggered) {
      // LED 闪烁（快速）
      static unsigned long lastBlink = 0;
      if (millis() - lastBlink > 100) {  // 每100ms闪烁一次
        lastBlink = millis();
        static bool ledState = false;
        ledState = !ledState;
        ledcWrite(LED_PIN, ledState ? 255 : 0);
      }
    }
  }
}