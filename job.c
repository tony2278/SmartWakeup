//核心程序（Arduino IDE）

  #include "BluetoothSerial.h"
  #include <SPIFFS.h>

  BluetoothSerial SerialBT;

  // 按钮引脚
  const int buttonPin = 0; // ESP32的Boot按键
  int lastButtonState = HIGH;

  // "小度小度"音频数据 (8kHz 8bit PCM)
  // 你需要准备音频文件，这里是示例占位
  const uint8_t xiaodu_xiaodu_audio[] = {
      // 这里放置转换为C数组的音频数据
      // 约200-300字节的8位音频数据
  };

  void setup() {
      Serial.begin(115200);
      pinMode(buttonPin, INPUT_PULLUP);

      // 初始化文件系统（如使用SPIFFS存储音频文件）
      if(!SPIFFS.begin(true)){
          Serial.println("SPIFFS mount failed");
          return;
      }

      // 蓝牙初始化
      SerialBT.begin("LanYa_Button"); // 蓝牙名称
      Serial.println("蓝牙已启动，请连接小度音箱");
  }

  void loop() {
      int buttonState = digitalRead(buttonPin);

      // 检测按钮按下（下降沿）
      if (buttonState == LOW && lastButtonState == HIGH) {
          delay(50); // 消抖
          if (digitalRead(buttonPin) == LOW) {
              playWakeWord();
          }
      }
      lastButtonState = buttonState;
  }

  void playWakeWord() {
      Serial.println("播放唤醒词...");

      // 方法1：直接发送音频数据到蓝牙
      // 实际应用中需要正确配置音频编码和采样率
      // SerialBT.write(xiaodu_xiaodu_audio, sizeof(xiaodu_xiaodu_audio));

      // 方法2：使用I2S/喇叭播放（让小度听到）
      // 播放存储在SPIFFS中的音频文件

      // ⚠️ 注意：实际蓝牙音频传输需要配置A2DP协议
      // 建议使用ESP32-A2DP库
  }