# 丰富语音控制功能 - 实施方案

## 背景

当前项目仅支持 SU-03T 语音模块的 3 个命令（0x00 播放提示音、0x01 开灯、0x02 关灯）。需要扩展到 12 个命令，新增天气播报、时间报时、室内温湿度播报和音乐播放控制功能。

## 需求映射

| 命令码 | 语音指令 | 功能                                     |
|--------|----------|------------------------------------------|
| 0x00   | 唤醒     | 播放唤醒提示音（已有，维持）             |
| 0x01   | 开灯     | LED亮 + 提示音（已有，维持）             |
| 0x02   | 关灯     | LED灭 + 提示音（已有，维持）             |
| 0x03   | 播报天气 | 调用 Weather_Voice_Play 播报当前天气     |
| 0x04   | 现在几点了 | 播报 RTC 时间：X年X月X号X点X分          |
| 0x05   | 房间温度多少 | 播报 DHT11 室内温度                    |
| 0x06   | 播报湿度 | 播报 DHT11 室内湿度                      |
| 0x07   | 播放音乐 | 播放文件夹 01 中的音乐                   |
| 0x08   | 上一首   | 切换到上一首                              |
| 0x09   | 下一首   | 切换到下一首                              |
| 0x0A   | 暂停     | 暂停播放（复用 stop 命令）               |
| 0x0B   | 停止     | 停止播放（MY1680 stop 命令）             |

## 设计方案

### 1. 扩展 `SU_03T_Control()` 命令表

**文件**: [HARDWARE/src/su03t.c](HARDWARE/src/su03t.c)

将 switch-case 从 3 个命令扩展到 12 个命令。需要新增：
- 音乐播放状态变量（当前曲目索引、总曲目数）
- 对 0x03~0x0B 的处理分支

新增代码位置：`su03t.c` 文件末尾，`SU_03T_Control()` 函数内。

音乐播放状态用 `static` 变量维护在 `su03t.c` 中，不污染全局命名空间。

### 2. 新增语音播报函数

**文件**: 
- [HARDWARE/src/weather_voice.c](HARDWARE/src/weather_voice.c) — 实现
- [HARDWARE/inc/weather_voice.h](HARDWARE/inc/weather_voice.h) — 声明

新增 3 个函数：

```c
void Voice_ReportTime(void);                // 报时
void Voice_ReportIndoorTemp(void);          // 报室内温度
void Voice_ReportIndoorHum(void);           // 报室内湿度
```

#### 报时逻辑 (Voice_ReportTime)
```
RTC_Analysis() -> rtc_time.year/month/day/hour/minute
播放顺序:
  年份数值 → 00/015(年) → 月份数值 → 00/016(月) → 日数值 → 00/017(号)
  → 小时数值 → 00/020(点) → 分钟数值 → 00/021(分)
```

#### 报室温逻辑 (Voice_ReportIndoorTemp)
```
读取 indoor_data.temp
播放顺序:
  00/012(室内温度前缀) → 温度数值 → 00/011(度)
```

#### 报湿度逻辑 (Voice_ReportIndoorHum)
```
读取 indoor_data.humi
播放顺序:
  00/013(室内湿度前缀) → 湿度数值 → 00/023(百分之)
```

**支持文件说明**（用户提供的文件夹 00 内容）：
- `00/012.mp3` — 室内温度（前缀）
- `00/013.mp3` — 室内湿度（前缀）
- `00/015.mp3` — 年
- `00/016.mp3` — 月
- `00/017.mp3` — 号
- `00/020.mp3` — 点
- `00/021.mp3` — 分

数字播报复用已有的 `MY1680_PlayNum()` 函数（支持 0~100+1位小数），已在 `weather_voice.c` 中实现。

### 3. 天气播报触发机制

**文件**: [USER/src/app.c](USER/src/app.c)

当前天气播报只在城市切换时在 `WeatherTask` 中触发。语音命令 0x03 需要从 `KeyTask`（运行 `SU_03T_Control()`）触发天气播报。

**方案**: 在已有队列命令 `CMD_SWITCH_CITY` (1) 基础上新增 `CMD_WEATHER_VOICE` (2)。`SU_03T_Control()` 通过 `xQueueSend()` 向 `weatherCmdQueue` 发送命令，`WeatherTask` 收到后直接调用 `Weather_Voice_Play()` 播报当前缓存的天气数据（无需重新请求 API）。

这就是不直接在 `SU_03T_Control()` 中调用 `Weather_Voice_Play()` 的原因：后者内部 `Wait_Play_Done()` 会阻塞调用任务长达数秒，而 KeyTask 需要保持响应以处理按键。

### 4. 音乐播放管理

**文件**: [HARDWARE/src/su03t.c](HARDWARE/src/su03t.c)

音乐文件在文件夹 01 中：`01/000dead_man.mp3`，`01/001富士山下-陈奕迅.mp3`

```c
static u8 current_track = 0;      // 当前曲目索引
static u8 total_tracks = 2;       // 总曲目数（根据实际文件数调整）
```

- 播放音乐 (0x07): `MY1680_Play(0x01, current_track)`
- 上一首 (0x08): `if(current_track > 0) current_track--; else current_track = total_tracks - 1;` + 播放
- 下一首 (0x09): `current_track = (current_track + 1) % total_tracks;` + 播放
- 暂停 (0x0A): 发送 stop 命令
- 停止 (0x0B): 发送 stop 命令

暂停和停止均使用 MY1680 的 stop 命令（`UART5_SendBuff(stop, 5)`），因为 MY1680 模块无独立暂停命令。

### 5. 涉及的文件与改动汇总

| 文件 | 改动内容 |
|------|----------|
| [HARDWARE/src/su03t.c](HARDWARE/src/su03t.c) | 扩展 `SU_03T_Control()` switch-case；新增音乐状态变量；包含 `weather_voice.h` 头文件；发送队列命令到 WeatherTask |
| [HARDWARE/src/weather_voice.c](HARDWARE/src/weather_voice.c) | 新增 `Voice_ReportTime()`、`Voice_ReportIndoorTemp()`、`Voice_ReportIndoorHum()` |
| [HARDWARE/inc/weather_voice.h](HARDWARE/inc/weather_voice.h) | 声明上述 3 个新函数 |
| [USER/src/app.c](USER/src/app.c) | 新增 `CMD_WEATHER_VOICE` 命令定义；`WeatherTask` 中增加 `CMD_WEATHER_VOICE` 处理分支 |

### 6. 验证方式

1. **编译验证**: 在 Keil MDK 中编译项目，确保 0 error 0 warning
2. **逐个命令测试**（通过 SU-03T 模块依次触发各语音命令）：
   - 0x03 播报天气 → 听到当前天气语音
   - 0x04 现在几点了 → 听到时间报时
   - 0x05 房间温度多少 → 听到室内温度
   - 0x06 播报湿度 → 听到室内湿度
   - 0x07 播放音乐 → 听到音乐播放
   - 0x08/0x09 → 能切换曲目
   - 0x0A/0x0B → 能停止播放
