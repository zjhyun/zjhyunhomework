#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "iQOO 13";
const char* password = "31415926Btx";
const int TOUCH_PIN = 4;  // 触摸传感器引脚

WebServer server(80);

String makePage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>作业9：实时传感器仪表盘</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 50px; }
    .dashboard {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
      padding: 40px;
      border-radius: 15px;
      max-width: 500px;
      margin: 0 auto;
      box-shadow: 0 10px 30px rgba(0,0,0,0.2);
    }
    .sensor-value {
      font-size: 72px;
      font-weight: bold;
      margin: 30px 0;
      text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
    }
    .label {
      font-size: 24px;
      margin-bottom: 20px;
    }
    .status {
      font-size: 16px;
      opacity: 0.8;
      margin-top: 30px;
    }
    .update-time {
      font-size: 14px;
      opacity: 0.6;
      margin-top: 10px;
    }
  </style>
</head>
<body>
  <div class="dashboard">
    <h1>📊 实时传感器仪表盘</h1>
    <div class="label">触摸传感器数值</div>
    <div class="sensor-value" id="sensorValue">0</div>
    <div class="status" id="status">正在连接...</div>
    <div class="update-time" id="updateTime">最后更新：--:--:--</div>
  </div>

  <script>
    // 更新传感器数值
    function updateSensor() {
      fetch('/sensor')
        .then(response => response.text())
        .then(data => {
          document.getElementById('sensorValue').innerText = data;
          document.getElementById('status').innerText = '数据已更新';
          
          // 更新时间
          const now = new Date();
          const timeStr = now.getHours().toString().padStart(2, '0') + ':' +
                         now.getMinutes().toString().padStart(2, '0') + ':' +
                         now.getSeconds().toString().padStart(2, '0');
          document.getElementById('updateTime').innerText = '最后更新：' + timeStr;
        })
        .catch(error => {
          document.getElementById('status').innerText = '连接失败';
        });
    }
    
    // 每500ms更新一次
    setInterval(updateSensor, 500);
    window.onload = updateSensor;
  </script>
</body>
</html>
)rawliteral";
  return html;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
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
  
  // 打印触摸传感器初始值
  Serial.print("初始触摸值: ");
  Serial.println(touchRead(TOUCH_PIN));
  
  // 设置 Web 服务器路由
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html; charset=UTF-8", makePage());
  });
  
  server.on("/sensor", HTTP_GET, []() {
    int touchValue = touchRead(TOUCH_PIN);
    server.send(200, "text/plain", String(touchValue));
  });
  
  server.begin();
  Serial.println(" 仪表盘已启动");
}

void loop() {
  server.handleClient();
}