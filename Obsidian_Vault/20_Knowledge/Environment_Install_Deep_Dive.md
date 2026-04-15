# 环境安装超详细手册（macOS + VSCode + STM32G474 + DAPLink）

## 0. 目标与完成标准

本手册目标是把环境搭到“工程可编译 + 可下载 + 可断点调试”的状态。

完成标准：

- 终端能运行 `arm-none-eabi-gcc`、`arm-none-eabi-gdb`、`openocd`、`make`。
- 工程可 `make -j8` 编译通过。
- 能用 OpenOCD 识别 `CMSIS-DAP` 和 `stm32g4x`。
- VSCode 按 `F5` 可启动调试并停在 `main`。

---

## 1. 准备阶段

### 1.1 硬件准备

- 开发板：`STM32G474CEU7` 对应板卡。
- 调试器：`DAPLink`（CMSIS-DAP v2）。
- 线材：支持数据传输的 USB 线（不是纯充电线）。
- 供电：保证板子电压稳定。

### 1.2 软件准备

- `VSCode`
- `STM32CubeMX`
- `Arm GNU Toolchain`（cask: `gcc-arm-embedded`）
- `openocd`

---

## 2. 安装步骤（从零开始）

### 2.1 安装 Homebrew（如果未安装）

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

安装后检查：

```bash
brew --version
```

### 2.2 安装 OpenOCD

```bash
brew install open-ocd
```

检查：

```bash
openocd --version
```

### 2.3 安装 Arm GNU Toolchain（推荐 cask 版）

```bash
brew install --cask gcc-arm-embedded
```

解释：

- cask 版是完整 ARM 官方工具链，含标准头文件和库。
- 之前常见报错 `stdint.h: No such file or directory`，多半是装到了不完整 formula 版本。

### 2.4 配置 PATH（重点）

本项目当前安装路径是：

`/Applications/ArmGNUToolchain/14.2.rel1/arm-none-eabi/bin`

写入 `~/.zshrc`：

```bash
echo 'export PATH="/Applications/ArmGNUToolchain/14.2.rel1/arm-none-eabi/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

验证：

```bash
which arm-none-eabi-gcc
arm-none-eabi-gcc --version
arm-none-eabi-gdb --version
```

---

## 3. VSCode 安装与扩展

### 3.1 推荐扩展

- `ms-vscode.cpptools`
- `marus25.cortex-debug`
- `ms-vscode.makefile-tools`（可选）

建议按扩展 ID 搜索，避免同名插件。

### 3.2 为什么这些扩展是必须/建议

- `cpptools`：提供 C/C++ 语义分析、跳转、错误提示。
- `cortex-debug`：负责嵌入式 GDB 调试 UI（断点、寄存器、外设、变量）。
- `makefile-tools`：可选，辅助触发 `make` 任务。

---

## 4. CubeMX 生成工程（Makefile 路线）

### 4.1 必要配置

1. 选芯片 `STM32G474CEU7`。
2. `SYS -> Debug = Serial Wire`（给 SWD 用）。
3. 先配置一个 LED 引脚输出（本项目是 `PC6`）。
4. `Project Manager -> Toolchain/IDE = Makefile`。
5. 生成代码到工程目录。

### 4.2 时钟建议（新手阶段）

- 先让系统“稳定跑起来”再优化频率。
- 如果不确定外部晶振，先保守配置。
- 时钟初始化失败时优先检查 `RCC` 设置与硬件是否匹配。

---

## 5. 命令行链路验证（分层检查）

### 5.1 编译链路

```bash
make -j8
```

成功标志：

- 生成 `build/*.elf`、`build/*.hex`、`build/*.bin`。

### 5.2 探针与芯片识别链路

```bash
openocd -f interface/cmsis-dap.cfg -f target/stm32g4x.cfg -c "init; shutdown"
```

成功日志关键词：

- `Using CMSIS-DAPv2 interface`
- `Cortex-M4 ... detected`

### 5.3 下载链路

```bash
openocd -f interface/cmsis-dap.cfg -f target/stm32g4x.cfg \
  -c "adapter speed 1000" \
  -c "program build/G474_Baremetal_Blink.elf verify reset exit"
```

---

## 6. 常见故障与对策

### 6.1 `command not found: arm-none-eabi-gcc`

原因：

- PATH 未生效，或工具链版本路径写错。

处理：

- `source ~/.zshrc`
- `which arm-none-eabi-gcc` 检查实际路径。

### 6.2 `stdint.h` 缺失

原因：

- 工具链不完整（无标准头文件）。

处理：

- 使用 `brew install --cask gcc-arm-embedded`。

### 6.3 `OpenOCD: GDB Server Quit Unexpectedly`

原因：

- 旧 `openocd` 进程占用探针。

处理：

```bash
pkill -f openocd
pkill -f arm-none-eabi-gdb
```

### 6.4 `external reset detected` 连续刷屏

说明：

- NRST 线上有反复复位信号。

排查：

- 线材、供电、复位电路、下载速度（先 1000kHz）。

---

## 7. 快速复盘清单（每次新机器部署）

- [ ] `brew --version` 正常
- [ ] `arm-none-eabi-gcc --version` 正常
- [ ] `openocd --version` 正常
- [ ] `make -j8` 编译通过
- [ ] `openocd ... init; shutdown` 识别成功
- [ ] `F5` 可调试并停在 `main`

