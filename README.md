# ESP32 蓝牙唤醒词播放器

通过 ESP32 实现自动播放"小度小度"唤醒词，从而远程唤醒小度智能音箱。

## 项目文件说明

| 文件 | 说明 | 使用场景 |
|------|------|----------|
| `project.ino` | A2DP 版本 | 通过蓝牙A2DP直接发送音频到小度音箱（推荐） |
| `job.ino` | I2S 版本 | 通过外接喇叭播放，小度通过麦克风接收 |
| `data/xiaoduxiaodu.wav` | 唤醒词音频文件 | 8kHz/16kHz 16bit PCM WAV 格式 |

## 快速开始

### 方案一：A2DP 蓝牙直接连接（推荐）

1. **安装 ESP32 开发板支持**
   - 打开 Arduino IDE
   - 文件 -> 首选项 -> 附加开发板管理器网址
   - 添加: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`

2. **安装 ESP32-A2DP 库**
   - 工具 -> 管理库 -> 搜索 "ESP32-A2DP" -> 安装

3. **配置项目**
   - 在 Arduino IDE 中打开 `project.ino`
   - 选择开发板: ESP32 Dev Module
   - 选择正确的串口

4. **首次配对（只需一次）**
   - 将代码上传到 ESP32
   - 在小度音箱上启用蓝牙配对模式
   - 在手机蓝牙设置中找到 `ESP32_WakeWord` 并连接
   - 配对完成后，ESP32 会自动连接小度音箱

5. **使用**
   - 按下 ESP32 的 BOOT 按钮即可播放唤醒词
   - 小度音箱应该会响应并进入语音交互模式

---

### 方案二：I2S 喇叭播放

1. **准备硬件**
   - ESP32 开发板
   - I2S 喇叭模块（如 MAX98357A）
   - 连接线

2. **硬件连接**
   | 引脚 | ESP32 GPIO |
   |------|------------|
   | WS (LRCK) | GPIO 15 |
   | SCK (BCLK) | GPIO 14 |
   | SD (DIN) | GPIO 2 |
   | VIN | 5V/3.3V |
   | GND | GND |

3. **上传音频文件到 SPIFFS**
   - 在 Arduino IDE 中打开 `job.ino`
   - 选择 工具 -> ESP32 Sketch Data Upload
   - 等待 `xiaoduxiaodu.wav` 上传完成

4. **上传代码**
   - 在 Arduino IDE 中点击上传按钮
   - 等待编译和上传完成

5. **使用**
   - 按下 BOOT 按钮播放唤醒词
   - 确保喇叭距离小度音箱足够近（约1米内）

---

## 音频文件要求

- **格式**: WAV (PCM)
- **采样率**: 8000Hz 或 16000Hz
- **位深度**: 16-bit
- **声道**: 单声道或立体声
- **内容**: "小度小度"唤醒词（约 1-2 秒）

### 转换音频文件

如果需要转换音频格式，可以使用以下工具：

```bash
# 使用 ffmpeg 转换（命令行）
ffmpeg -i input.mp3 -ar 16000 -ac 1 -f wav xiaoduxiaodu.wav
```

---

## 代码修改

### 修改蓝牙设备名称

在 `project.ino` 中修改设备名称：

```cpp
a2dp_source.start("Your_Custom_Name");
```

### 修改 I2S 引脚

在 `job.ino` 中修改引脚定义：

```cpp
#define I2S_WS_PIN 15      // 修改为你的 WS 引脚
#define I2S_SCK_PIN 14     // 修改为你的 SCK 引脚
#define I2S_SD_PIN 2       // 修改为你的 SD 引脚
```

---

## 故障排除

### A2DP 版本

| 问题 | 解决方案 |
|------|----------|
| 无法连接小度音箱 | 确保小度音箱处于配对模式，并在手机蓝牙中手动配对一次 |
| 播放没有声音 | 检查蓝牙连接状态，确保 A2DP 配置正确 |
| 按钮无响应 | 检查按钮引脚是否正确（默认为 GPIO 0） |

### I2S 版本

| 问题 | 解决方案 |
|------|----------|
| 无法打开音频文件 | 使用 "ESP32 Sketch Data Upload" 上传 WAV 文件 |
| 播放杂音 | 检查 I2S 引脚连接，确保采样率匹配 |
| 喇叭不响 | 检查电源供应，喇叭模块是否正确连接 |

---

## 项目结构

```
LanYa/
├── project.ino          # A2DP 蓝牙版本
├── job.ino              # I2S 喇叭版本
├── project.c            # A2DP C 语言版本（参考）
├── job.c                # I2S C 语言版本（参考）
├── data/
│   └── xiaoduxiaodu.wav # 唤醒词音频文件
├── xiaoduxiaodu.wav     # 原始音频文件
└── README.md            # 本文件
```

---

## 许可证

MIT License

---

## 作者

LanYa 项目

---

## 更新日志

- v1.0 (2026-02-27)
  - 初始版本
  - 支持 A2DP 和 I2S 两种方案
  - 添加完整的文档和使用说明
