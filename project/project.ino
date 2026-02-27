/**
 * ESP32 蓝牙唤醒词播放器 - A2DP 版本
 * 功能：通过蓝牙A2DP将"小度小度"唤醒词发送到小度音箱
 *
 * 依赖库：ESP32-A2DP (通过 Arduino Library Manager 安装)
 *
 * 使用说明：
 * 1. 安装 ESP32 开发板支持
 * 2. 安装 ESP32-A2DP 库
 * 3. 上传代码到 ESP32
 * 4. 首次使用需手动配对小度音箱
 * 5. 按下 BOOT 按钮播放唤醒词
 */

#include "BluetoothA2DP.h"

BluetoothA2DPSource a2dp_source;

// 全局音频状态
bool isPlaying = false;
unsigned long playStartTime = 0;
const unsigned long WAKE_WORD_DURATION = 2000; // 唤醒词播放时长 (ms)

// 按钮引脚 (ESP32 Boot按键: GPIO0)
const int buttonPin = 0;
int lastButtonState = HIGH;

// "小度小度"音频样本 - 简单的音频波形
// 实际项目中应该将 WAV 文件转换为 C 数组
#define SINE_WAVE_SAMPLES 100
const int16_t sine_wave[SINE_WAVE_SAMPLES] = {
    0, 2057, 4106, 6140, 8149, 10125, 12059, 13943, 15768, 17526,
    19209, 20811, 22324, 23743, 25062, 26275, 27379, 28369, 29242, 29995,
    30625, 31131, 31512, 31767, 31896, 31901, 31781, 31538, 31173, 30688,
    30086, 29370, 28543, 27609, 26571, 25433, 24200, 22877, 21469, 19982,
    18423, 16797, 15111, 13372, 11585, 9757, 7944, 6061, 4118, 2124,
    88, -1952, -3976, -5972, -7934, -9855, -11727, -13542, -15292, -16969,
    -18566, -20078, -21498, -22820, -24040, -25153, -26155, -27044, -27815, -28467,
    -28996, -29402, -29684, -29841, -29873, -29780, -29563, -29224, -28764, -28185,
    -27491, -26683, -25766, -24744, -23620, -22399, -21086, -19686, -18205, -16648,
    -15022, -13332, -11584, -9785, -7941, -6058, -4146, -2213, -269, 1676
};

/**
 * 音频数据回调函数
 * A2DP库会定期调用此函数获取音频数据
 */
int32_t get_audio_data(Frame *frame, int32_t frame_count) {
    static uint32_t sampleIndex = 0;

    // 检查是否正在播放
    if (!isPlaying) {
        // 发送静音
        for (int i = 0; i < frame_count; i++) {
            frame[i].channel1 = 0;
            frame[i].channel2 = 0;
        }
        return frame_count;
    }

    // 检查播放时间
    if (millis() - playStartTime > WAKE_WORD_DURATION) {
        isPlaying = false;
        sampleIndex = 0;
        for (int i = 0; i < frame_count; i++) {
            frame[i].channel1 = 0;
            frame[i].channel2 = 0;
        }
        Serial.println("播放完成");
        return frame_count;
    }

    // 填充音频数据
    for (int i = 0; i < frame_count; i++) {
        int32_t sample = (int32_t)(sine_wave[(sampleIndex++) % SINE_WAVE_SAMPLES] * 100);
        frame[i].channel1 = sample;
        frame[i].channel2 = sample;
    }

    return frame_count;
}

/**
 * 开始播放唤醒词
 */
void playWakeWord() {
    Serial.println("开始播放: 小度小度");
    isPlaying = true;
    playStartTime = millis();
}

/**
 * 初始化设置
 */
void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n========================================");
    Serial.println("  ESP32 蓝牙唤醒词播放器 (A2DP版)");
    Serial.println("  版本: 1.0");
    Serial.println("========================================\n");

    // 初始化按钮
    pinMode(buttonPin, INPUT_PULLUP);
    lastButtonState = digitalRead(buttonPin);

    // 设置音频回调
    a2dp_source.set_data_callback(get_audio_data);

    // 启动蓝牙
    Serial.println("启动蓝牙A2DP源...");
    Serial.println("蓝牙设备名称: ESP32_WakeWord");
    Serial.println("\n首次使用请手动配对小度音箱:");
    Serial.println("  1. 在小度音箱上启用蓝牙配对");
    Serial.println("  2. 在手机蓝牙设置中找到 ESP32_WakeWord 并连接");
    Serial.println("  3. 配对完成后即可使用\n");

    a2dp_source.start("ESP32_WakeWord");

    Serial.println("========================================");
    Serial.println("  系统已启动!");
    Serial.println("========================================");
    Serial.println("\n操作说明:");
    Serial.println("  - 按下 BOOT 按钮播放唤醒词");
    Serial.println("  - 确保已连接小度音箱\n");
}

/**
 * 主循环
 */
void loop() {
    // 读取按钮状态
    int buttonState = digitalRead(buttonPin);

    // 检测按钮按下（下降沿触发）
    if (buttonState == LOW && lastButtonState == HIGH) {
        delay(50); // 软件消抖
        if (digitalRead(buttonPin) == LOW) {
            playWakeWord();
        }
    }

    lastButtonState = buttonState;
    delay(10);
}
