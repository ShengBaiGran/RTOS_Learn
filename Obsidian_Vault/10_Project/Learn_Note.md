# STM32G474CEU7 + FreeRTOS 学习总笔记（Obsidian）

## 0. 当前状态

- 工程：`G474_Baremetal_Blink`
- 编译：OK
- 下载调试：OK（`DAPLink v2 + OpenOCD + Cortex-Debug`）
- LED 点灯：OK（`PC6`）
- UART 交互命令：OK（`on/off/toggle/status/help`）
- Epaper 2.13 三色：OK（`SPI2`）
- 默认自动刷屏压测：ON（`1000 ms` 周期）

---

## 1. 开发环境（macOS + VSCode）

### 1.1 工具链（当前已跑通）

- `arm-none-eabi-gcc`：Arm GNU Toolchain 14.2.Rel1
- `arm-none-eabi-gdb`：Arm GNU Toolchain 14.2.Rel1
- `openocd`：0.12.0
- `make`：系统自带

### 1.2 终端验证

```bash
arm-none-eabi-gcc --version
arm-none-eabi-gdb --version
make --version
openocd --version
```

### 1.3 VSCode 扩展（按 ID 搜索）

- `ms-vscode.cpptools`
- `marus25.cortex-debug`
- `ms-vscode.makefile-tools`（可选）
- `ms-vscode.vscode-embedded-tools`（可选）

### 1.4 关键路径

- 工具链路径：`/Applications/ArmGNUToolchain/14.2.rel1/arm-none-eabi/bin`
- 调试文件：`/.vscode/launch.json`、`/.vscode/tasks.json`
- SVD 文件：`/.vscode/cmsis-svd-stm32/stm32g4/STM32G474.svd`

### 1.5 深度学习入口（建议先看）

- 环境安装超详细手册：[Environment_Install_Deep_Dive.md](../20_Knowledge/Environment_Install_Deep_Dive.md)
- VSCode 配置文件原理：[VSCode_Config_Principles.md](../20_Knowledge/VSCode_Config_Principles.md)

---

## 2. CubeMX 工程创建教程（Makefile）

### 2.1 创建步骤

1. CubeMX 新建工程，芯片选 `STM32G474CEU7`。
2. `SYS -> Debug` 设为 `Serial Wire`。
3. 先配置一个 LED 引脚为输出（本工程使用 `PC6`）。
4. `Project Manager -> Toolchain/IDE` 选 `Makefile`。
5. 生成代码并用 VSCode 打开工程目录。

### 2.2 时钟配置建议（入门优先稳定）

- 先用简单稳定配置跑通“编译 + 下载 + 闪灯”。
- `HSE/LSE` 只有在你确认板上硬件存在并匹配时再启用。
- `HSI48` 常用于 USB；不做 USB 时可先不依赖它。

### 2.3 RCC 页面常见项

- `MCO`：时钟输出到引脚（调试测频用）
- `LSCO`：低速时钟输出
- `I2S_CKIN`：外部音频时钟输入
- `CRS`：给 `HSI48` 做时钟恢复/校准（USB 场景常用）

---

## 3. 编译、下载、调试（可执行）

### 3.1 编译

```bash
make -j8
```

### 3.2 命令行下载

```bash
openocd -f interface/cmsis-dap.cfg -f target/stm32g4x.cfg \
  -c "adapter speed 1000" \
  -c "program build/G474_Baremetal_Blink.elf verify reset exit"
```

### 3.3 命令行调试（双终端）

终端 1：

```bash
openocd -f interface/cmsis-dap.cfg -f target/stm32g4x.cfg
```

终端 2：

```bash
arm-none-eabi-gdb build/G474_Baremetal_Blink.elf
```

```gdb
target extended-remote :3333
monitor reset halt
load
monitor reset run
quit
```

### 3.4 VSCode 图形化调试

- 直接 `F5` 使用 `STM32G474 Launch (DAPLink + OpenOCD)`。
- 需要排硬故障时，选 `STM32G474 Launch (Fault Breakpoints)`。
- 如果你手动启动了 OpenOCD，使用 `Attach` 配置。

---

## 4. 本项目已踩坑与修复

### 4.1 `crdb_full.zip` 下载失败

- 现象：CubeMX 更新/下载中断。
- 处理：清缓存后重试，必要时切网络/代理。

### 4.2 `stdint.h` 缺失

- 现象：编译报 `fatal error: stdint.h: No such file or directory`。
- 根因：装到了不完整 `arm-none-eabi-gcc` formula（`--without-headers`）。
- 处理：改用 `gcc-arm-embedded` cask 完整工具链。

### 4.3 工具链命令找不到

- 现象：`arm-none-eabi-gcc: command not found`。
- 根因：`PATH` 写成了 `15.2.rel1`，实际安装为 `14.2.rel1`。
- 处理：修正 `~/.zshrc` 到正确路径。

### 4.4 `OpenOCD: GDB Server Quit Unexpectedly`

- 根因：已有 `openocd` 在后台占用探针。
- 处理：

```bash
pkill -f openocd
pkill -f arm-none-eabi-gdb
```

### 4.5 `external reset detected` 连续刷屏

- 含义：NRST 线上反复复位信号。
- 排查：线材、供电、复位电路、适配器速度（先降到 `1000`）。

---

## 5. FreeRTOS 学习路线（打勾推进）

- [ ] 裸机工程稳定运行（GPIO/UART）
- [ ] 加入 FreeRTOS 内核并成功启动调度器
- [ ] 创建 2~3 个基础任务（LED、串口、心跳）
- [ ] 验证优先级与时间片行为
- [ ] 使用 Queue 完成任务通信
- [ ] 使用 Semaphore 做同步
- [ ] 使用 Software Timer
- [ ] 处理栈溢出与断言配置

---

## 6. 每日记录与问题记录

- 每日记录：使用 [Daily_Note_Template.md](../99_Templates/Daily_Note_Template.md)
- 故障复盘：使用 [Issue_Template.md](../99_Templates/Issue_Template.md)
- 调试速查：[Debug_Workflow.md](../20_Knowledge/Debug_Workflow.md)
- 时钟与复位：[Clock_and_Reset.md](../20_Knowledge/Clock_and_Reset.md)

---

## 7. 里程碑

- [x] M1：基础移植成功（编译 + 下载 + 点灯）
- [x] M1.5：串口命令行 + Epaper 2.13 三色接入
- [ ] M2：任务通信机制跑通
- [ ] M3：稳定运行 24h

---

## 8. Epaper 2.13 三色（WeAct）接入记录

### 8.1 引脚映射（当前工程）

- `PB13` -> `SPI2_SCK`
- `PB15` -> `SPI2_MOSI`
- `PB12` -> `CS`（GPIO）
- `PB14` -> `DC`（GPIO）
- `PA10` -> `RES`（GPIO）
- `PA9` -> `BUSY`（GPIO 输入）

### 8.2 当前功能

- 已支持命令：`epdinit / epdclear / epdtest / epdtext / epdscroll / epdbusy / epdsleep`
- 已支持自动刷屏长测：上电默认开启
- 自动刷屏周期：`#define EPD_AUTO_REFRESH_INTERVAL_MS 1000U`
- 自动刷屏开关命令：`epdautoon / epdautooff`

### 8.3 长测建议

- 建议至少跑 `24h`，观察：
- `BUSY` 是否偶发超时
- 是否出现鬼影残留异常
- 局部区域是否出现对比度衰减
- 建议每次改刷新策略后，重新做一次 `24h` 对比
