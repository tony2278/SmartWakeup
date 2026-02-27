/**
 * ESP32 蓝牙唤醒词播放器 - I2S/喇叭版本
 * 功能：通过I2S接口播放"小度小度"唤醒词到外接喇叭
 * 小度通过麦克风听到喇叭声音
 *
 * 依赖库：
 * - 无需额外库，使用 ESP32 自带 I2S 驱动
 *
 * 使用说明：
 * 1. 将 xiaoduxiaodu.wav 放入 data 文件夹
 * 2. 使用 Arduino IDE 的 "ESP32 Sketch Data Upload" 上传音频文件
 * 3. 上传代码到 ESP32
 * 4. 连接 I2S 喇叭模块
 * 5. 按下 BOOT 按钮播放唤醒词
 */

#include <Arduino.h>
#include <SPIFFS.h>
#include <driver/i2s.h>

// 按钮引脚 (ESP32 Boot按键: GPIO0)
const int buttonPin = 0;
int lastButtonState = HIGH;

// I2S 配置 (根据实际硬件连接修改)
#define I2S_WS_PIN 15      // Word Select (LRCK)
#define I2S_SD_PIN 2       // Serial Data (DIN)
#define I2S_SCK_PIN 14     // Serial Clock (BCLK)
#define I2S_PORT I2S_NUM_0

// 音频参数
#define SAMPLE_RATE 16000
#define BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT
#define CHANNEL_COUNT 2

// 播放状态
volatile bool shouldPlay = false;
unsigned long playStartTime = 0;
const unsigned long WAKE_WORD_DURATION = 2000; // ms

// SPIFFS 音频文件路径
const char* WAKE_WORD_FILE = "/xiaoduxiaodu.wav";

/**
 * 初始化 I2S 音频接口
 */
void initI2S() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = BITS_PER_SAMPLE,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_SD_PIN,
        .data_in_num = I2S_PIN_NO_CHANGE,
    };

    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT, &pin_config);
    i2s_zero_dma_buffer(I2S_PORT);
}

/**
 * 播放 WAV 文件
 * @param filename SPIFFS中的文件名
 */
void playWAVFile(const char* filename) {
    File file = SPIFFS.open(filename, "r");

    if (!file) {
        Serial.println("❌ 无法打开音频文件: " + String(filename));
        Serial.println("   请确保已通过 'ESP32 Sketch Data Upload' 上传音频文件");
        return;
    }

    Serial.println("▶ 开始播放: " + String(filename));

    // 读取 WAV 文件头 (44字节)
    uint8_t header[44];
    file.read(header, 44);

    // 简单验证是否为 WAV 文件
    if (header[0] != 'R' || header[1] != 'I' || header[2] != 'F' || header[3] != 'F') {
        Serial.println("❌ 不是有效的WAV文件!");
        file.close();
        return;
    }

    // 跳过 WAV 头，定位到数据区
    file.seek(44);

    // 读取音频数据并播放
    size_t bytesRead;
    uint8_t buffer[512];
    playStartTime = millis();
    unsigned long bytesPlayed = 0;

    while (millis() - playStartTime < WAKE_WORD_DURATION && file.available()) {
        bytesRead = file.read(buffer, sizeof(buffer));

        if (bytesRead > 0) {
            size_t bytesWritten;
            i2s_write(I2S_PORT, buffer, bytesRead, &bytesWritten, portMAX_DELAY);
            bytesPlayed += bytesWritten;
        }
    }

    // 播放静音避免杂音
    uint8_t silence[256] = {0};
    i2s_write(I2S_PORT, silence, sizeof(silence), NULL, 100);

    file.close();
    Serial.println("✓ 播放完成 (共播放 " + String(bytesPlayed) + " 字节)");
}

/**
 * 检查并上传音频文件到 SPIFFS
 * 首次运行时调用
 */
void setupAudioFile() {
    Serial.println("\n--- 音频文件检查 ---");

    if (!SPIFFS.exists(WAKE_WORD_FILE)) {
        Serial.println("⚠ 音频文件不存在: " + String(WAKE_WORD_FILE));
        Serial.println("\n请按以下步骤操作:");
        Serial.println("  1. 在项目目录创建 'data' 文件夹");
        Serial.println("  2. 将 xiaoduxiaodu.wav 复制到 data 文件夹");
        Serial.println("  3. 在 Arduino IDE 中选择: 工具 -> ESP32 Sketch Data Upload");
        Serial.println("  4. 重新上传代码\n");
    } else {
        File file = SPIFFS.open(WAKE_WORD_FILE, "r");
        Serial.println("✓ 音频文件已就绪: " + String(WAKE_WORD_FILE));
        Serial.println("  文件大小: " + String(file.size()) + " 字节");
        file.close();
    }
    Serial.println("-------------------\n");
}

/**
 * 按钮检测并触发播放
 */
void checkButton() {
    int buttonState = digitalRead(buttonPin);

    // 检测按钮按下（下降沿触发）
    if (buttonState == LOW && lastButtonState == HIGH) {
        delay(50); // 软件消抖
        if (digitalRead(buttonPin) == LOW) {
            Serial.println("\n🔔 按钮被按下!");
            playWAVFile(WAKE_WORD_FILE);
        }
    }

    lastButtonState = buttonState;
}

/**
 * 初始化设置
 */
void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n========================================");
    Serial.println("  ESP32 蓝牙唤醒词播放器 (I2S版)");
    Serial.println("  版本: 1.0");
    Serial.println("========================================\n");

    // 初始化按钮
    pinMode(buttonPin, INPUT_PULLUP);
    lastButtonState = digitalRead(buttonPin);

    // 初始化 SPIFFS 文件系统
    Serial.println("挂载 SPIFFS 文件系统...");
    if (!SPIFFS.begin(true)) {
        Serial.println("❌ SPIFFS 挂载失败!");
        return;
    }
    Serial.println("✓ SPIFFS 挂载成功");

    // 检查/设置音频文件
    setupAudioFile();

    // 初始化 I2S 音频接口
    Serial.println("初始化 I2S 音频接口...");
    initI2S();
    Serial.println("✓ I2S 初始化完成");

    Serial.println("========================================");
    Serial.println("  系统已启动!");
    Serial.println("========================================");
    Serial.println("\n操作说明:");
    Serial.println("  - 按下 BOOT 按钮播放唤醒词");
    Serial.println("  - 确保小度音箱在麦克风范围内");
    Serial.println("\n硬件连接 (I2S):");
    Serial.println("  WS (LRCK)  -> GPIO " + String(I2S_WS_PIN));
    Serial.println("  SCK (BCLK) -> GPIO " + String(I2S_SCK_PIN));
    Serial.println("  SD (DIN)   -> GPIO " + String(I2S_SD_PIN) + "\n");
}

/**
 * 主循环
 */
void loop() {
    checkButton();
    delay(10);
}
