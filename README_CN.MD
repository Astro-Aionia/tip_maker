# TipMaker - STM32 步进电机控制器

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![STM32F103C8T6](https://img.shields.io/badge/STM32-STM32F103C8T6-blue)](https://www.st.com/zh/microcontrollers-microprocessors/stm32f103c8.html)

基于 STM32F103C8T6 (Blue Pill) 的高级步进电机控制系统，具有 USB CDC 通信、INA236 电流监测和可编程运动序列功能。

## 目录
- [功能特性](#功能特性)
- [硬件要求](#硬件要求)
- [软件要求](#软件要求)
- [项目结构](#项目结构)
- [快速开始](#快速开始)
  - [1. 克隆仓库](#1-克隆仓库)
  - [2. 安装工具链](#2-安装工具链)
  - [3. 使用 CubeMX 生成代码](#3-使用-cubemx-生成代码)
  - [4. 编译项目](#4-编译项目)
  - [5. 烧录到 STM32](#5-烧录到-stm32)
- [硬件连接](#硬件连接)
- [串口命令参考](#串口命令参考)
  - [命令格式](#命令格式)
  - [可用命令](#可用命令)
  - [JSON 响应格式](#json-响应格式)
- [配置参数](#配置参数)
- [故障排除](#故障排除)
- [许可证](#许可证)

## 功能特性

- **USB CDC 通信**：虚拟 COM 端口，便于 PC 通信
- **步进电机控制**：精确的 PWM 控制，带硬件脉冲计数
- **电流监测**：INA236 I2C 电流传感器，8 样本缓冲区
- **可编程序列**：状态机实现自动化操作
- **非易失存储**：EEPROM 模拟实现参数持久化
- **JSON 接口**：所有响应采用标准 JSON 格式
- **模块化架构**：关注点分离，便于维护

## 硬件要求

1. **主控制器**：
   - STM32F103C8T6 (Blue Pill) 开发板

2. **步进电机驱动器**：
   - 任何接受 CW/CCW 脉冲的步进驱动器
   - 5V 兼容控制信号

3. **电流检测**：
   - INA236 电流传感器 (I2C 接口)

4. **电源供应**：
   - 3.3V 用于 STM32
   - 适当的步进电机电压

5. **连接性**：
   - USB Type-A 转 Micro-USB 电缆
   - 逻辑电平转换器（如需要）

## 软件要求

1. **STM32CubeMX** (v6.0+)：用于初始项目配置
2. **ARM GNU 工具链**：
   - gcc-arm-none-eabi
   - make
3. **OpenOCD**：用于烧录和调试
4. **串口终端**：
   - PuTTY (Windows)
   - screen/minicom (Linux/Mac)
   - CoolTerm (跨平台)

## 项目结构

```
tip_maker/
├── Core/                 # STM32 核心文件（由 CubeMX 生成）
│   ├── Inc/             # 头文件
│   ├── Src/             # 源文件
│   └── Startup/         # 启动代码
├── Drivers/             # HAL 和 CMSIS 驱动
├── App/                 # 应用程序代码
│   ├── Inc/             # 应用头文件
│   │   ├── stepper_motor.h
│   │   ├── ina236.h
│   │   ├── command_parser.h
│   │   ├── sequence_controller.h
│   │   ├── eeprom_emulation.h
│   │   └── system_state.h
│   └── Src/             # 应用源文件
├── Makefile             # 构建配置
├── README.md            # 英文文档
└── README_CN.md         # 中文文档
```

## TODO

- 修复round计数功能
- 制作PCB
- 建立文档

## 快速开始

### 1. 克隆仓库
```bash
git clone https://github.com/Astro-Aionia/tip_maker.git
cd tip_maker
```

### 2. 安装工具链

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install make gcc-arm-none-eabi openocd
```

**macOS (Homebrew):**
```bash
brew install arm-none-eabi-gcc make openocd
```

**Windows:**
1. 下载并安装 [ARM GNU 工具链](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm)
2. 安装 [Make for Windows](http://gnuwin32.sourceforge.net/packages/make.htm)
3. 安装 [OpenOCD](https://gnutoolchains.com/arm-eabi/openocd/)

### 3. 使用 CubeMX 生成代码
1. 打开 STM32CubeMX
2. 加载 `tip_maker.ioc`（如果存在）
3. 或使用 STM32F103C8Tx 创建新项目
4. 按照配置指南进行配置
5. 将 Toolchain/IDE 设置为 "Makefile" 并生成代码

### 4. 编译项目
```bash
make all
```
成功编译后将创建 `build/tip_maker.elf` 和 `build/tip_maker.bin`。

### 5. 烧录到 STM32

**使用 ST-Link:**
```bash
make flash
```
或手动执行：
```bash
openocd -f interface/stlink-v2.cfg -f target/stm32f1x.cfg \
  -c "program build/tip_maker.elf verify reset exit"
```

**使用 USB DFU:**
1. 设置 BOOT0=1, BOOT1=0
2. 复位开发板
3. 使用 DFU 工具烧录

## 硬件连接

| STM32 引脚 | 功能 | 连接 | 备注 |
|------------|------|------|------|
| PA8 | PWM_CW | 驱动器 CW 输入 | 5V 容忍 |
| PA9 | PWM_CCW | 驱动器 CCW 输入 | 5V 容忍 |
| PA0 | SWITCH_CURRENT | 二极管开关控制 | 高电平有效 |
| PA1 | SWITCH_HOLDOFF | 电机励磁控制 | ON=False, OFF=True |
| PA2 | SWITCH_DIVISION | 细分选择 | 高电平有效 |
| PA3 | INPUT_ROUNDOUT | 电机圈数输出 | 上拉输入 |
| PA4 | INPUT_ZERO | 零点指示 | 上拉输入 |
| PB6 | I2C1_SCL | INA236 SCL | 400kHz 快速模式 |
| PB7 | I2C1_SDA | INA236 SDA | 400kHz 快速模式 |
| PA11 | USB_DM | USB D- | |
| PA12 | USB_DP | USB D+ | |

## 串口命令参考

### 命令格式
```
COMMAND [PARAMETER] [VALUE]
```
- 所有命令均为 ASCII 字符串
- 参数由空格分隔
- 以 Enter/Return (CR/LF) 结束命令
- 命令区分大小写

### 可用命令

#### 1. SET - 配置参数
```
SET {Key} {Value}
```
**键和值：**
- `FREQ`：0-1000 Hz (整数)
- `THRES`：电流阈值 (微安 整数)
- `CURRENT`：ON/OFF (二极管开关)
- `HOLDOFF`：ON/OFF (电机励磁，ON=False, OFF=True)
- `DIVISION`：ON/OFF (细分选择)

**示例：**
```
SET FREQ 500
SET THRES 20
SET CURRENT ON
```

#### 2. GET - 读取参数
```
GET {Key}
```
**示例：**
```
GET FREQ
GET THRES
```

#### 3. MOVE - 控制电机运动
```
MOVE {Dir} {Step}
```
- `Dir`：CW 或 CCW
- `Step`：脉冲数 (1-65535)

**示例：**
```
MOVE CW 1000
MOVE CCW 500
```

#### 4. POLL - 读取电流缓冲区
```
POLL
```
返回缓冲区中的所有 8 个电流样本。

#### 5. START - 开始序列操作
```
START
```
触发自动化序列操作。

#### 6. STATUS - 获取系统状态
```
STATUS {Level}
```
- `Level`：1、2 或 3
  - 1：用户可配置参数
  - 2：级别 1 + 最后电流读数
  - 3：完整系统状态

**示例：**
```
STATUS 3
```

#### 7. DEBUG - 启用调试模式
```
DEBUG {Level}
```
- `Level`：0-3
  - 0：禁用调试输出
  - 1-3：与 STATUS 级别相同，每秒重复一次

**示例：**
```
DEBUG 2
DEBUG 0
```

### JSON 响应格式

所有响应都遵循以下结构：
```json
{
  "Cmd": "COMMAND_NAME",
  "Status": "Success/Error",
  ... 其他字段 ...
}
```

#### 响应示例：

**成功的 SET：**
```json
{
  "Cmd": "SET",
  "Status": "Success",
  "Parameter": "FREQ",
  "Value": "500"
}
```

**成功的 MOVE：**
```json
{
  "Cmd": "MOVE",
  "Status": "Success",
  "Direction": "CW",
  "Step": 1000
}
```

**STATUS 级别 3：**
```json
{
  "Cmd": "STATUS",
  "Status": "Success",
  "Level": 3,
  "FREQ": 500,
  "THRES": 20,
  "CURRENT": "ON",
  "HOLDOFF": "OFF",
  "DIVISION": "ON",
  "LastCurrent": 0.045,
  "MotorMoving": false,
  "TargetSteps": 1000,
  "CurrentSteps": 1000,
  "CurrentDirection": "S",
  "RoundCount": 5,
  "ZeroPoint": false
}
```

**错误响应：**
```json
{
  "Cmd": "SET",
  "Status": "Error",
  "Parameter": "FREQ",
  "Value": "1500"
}
```

## 配置参数

系统维护以下参数：

| 参数 | 类型 | 范围 | 默认值 | 描述 |
|------|------|------|--------|------|
| origin_freq | uint16_t | 1000 Hz | 1000 | 固定 PWM 频率 |
| freq | uint16_t | 0-1000 Hz | 500 | 用户可调频率 |
| threshold | int16_t | - | 0.1 | 电流阈值 (A) |
| switch_current | bool | ON/OFF | OFF | 二极管开关状态 |
| switch_holdoff | bool | ON/OFF | OFF | 电机励磁状态 |
| switch_division | bool | ON/OFF | OFF | 细分选择 |
| buffer_size | uint8_t | 8 | 8 | 电流缓冲区大小 |
| round_count | uint16_t | 0-65535 | 0 | 电机圈数计数器 |
| zero_point | bool | true/false | false | 零点位置指示器 |

## 故障排除

### 常见问题：

1. **USB 未检测到**：
   - 检查 USB 电缆连接
   - 安装适当的 USB CDC 驱动程序
   - 确保电源供应正常

2. **I2C 通信失败**：
   - 验证 SDA/SCL 上的上拉电阻 (4.7kΩ)
   - 检查 INA236 地址（默认 0x40）
   - 验证 INA236 的电源

3. **电机不转动**：
   - 使用示波器检查 PWM 信号
   - 验证电机驱动器电源
   - 确认控制信号电平

4. **编译错误**：
   - 确保 ARM 工具链在 PATH 中
   - 检查 Makefile 配置
   - 验证 CubeMX 生成的文件

5. **烧录失败**：
   - 检查 ST-Link 连接
   - 验证 BOOT 引脚设置
   - 尝试不同的 USB 端口/电缆

### 调试提示：

1. 启用调试模式以获取详细输出：
   ```
   DEBUG 3
   ```

2. 检查所有参数：
   ```
   STATUS 3
   ```

3. 监视电流读数：
   ```
   POLL
   ```

## 许可证

本项目采用 MIT 许可证 - 有关详细信息，请参阅 LICENSE 文件。

## 致谢

- STMicroelectronics 提供的 STM32CubeMX 和 HAL 库
- ARM 提供的 GNU 工具链
- OpenOCD 开发者
- 所有贡献者和测试人员

## 支持

遇到问题和疑问时：
1. 查看故障排除部分
2. 查看已打开/关闭的 GitHub 问题
3. 创建包含详细信息的新问题

---

*项目维护者：[Aionia/PKU]*