# STM32G474CEU7 + FreeRTOS 学习笔记

## 0. Mac 开发环境搭建（先完成）
### 0.1 推荐方案
- 编辑器：`VSCode`
- 工程生成：`STM32CubeMX`（生成 `Makefile` 工程）
- 编译工具链：`arm-none-eabi-gcc`
- 烧录调试：`openocd + DAPLink(CMSIS-DAP)`

### 0.2 安装清单（macOS）
- [ ] 安装 `VSCode`
- [ ] 安装 `STM32CubeMX`
- [ ] 安装 `ARM GNU Toolchain`（`arm-none-eabi-gcc`）
- [ ] 安装 `openocd`（`brew install open-ocd`）
- [ ] 安装 `make`（通常系统已自带）
- [ ] 准备数据线（支持数据传输，不是纯充电线）

### 0.3 VSCode 扩展（建议）
- [ ] `C/C++`（Microsoft，ID: `ms-vscode.cpptools`）
- [ ] `Cortex-Debug`（marus25，ID: `marus25.cortex-debug`）
- [ ] `Makefile Tools`（Microsoft，ID: `ms-vscode.makefile-tools`，可选）
- [ ] `Embedded Tools`（Microsoft，ID: `ms-vscode.vscode-embedded-tools`，可选）
- 搜索建议：优先按扩展 ID 搜索，避免装错同名插件

### 0.4 终端验证（安装后执行）
```bash
arm-none-eabi-gcc --version
arm-none-eabi-gdb --version
make --version
openocd --version
```
- 期望结果：四条命令都可执行
- 如果命令找不到：补充 `PATH`，确保工具链可在终端直接调用

### 0.5 创建第一个 STM32 工程（CubeMX -> Makefile）
1. 打开 CubeMX，新建工程并选择芯片：`STM32G474CEU7`
2. 配置时钟与引脚，先启用一个 LED 引脚（如 `PA5`，按板子实际引脚）
3. `Project Manager -> Toolchain / IDE` 选择 `Makefile`
4. 生成代码到当前工作目录
5. 用 VSCode 打开该工程目录

### 0.6 最小编译与下载（终端）
```bash
make -j8
openocd -f interface/cmsis-dap.cfg -f target/stm32g4x.cfg
```
- 新开一个终端执行（烧录示例）：
```bash
arm-none-eabi-gdb build/<your_project>.elf
```
- 在 gdb 里执行：
```gdb
target extended-remote :3333
monitor reset halt
load
monitor reset run
quit
```

### 0.7 VSCode 调试（可选）
- 后续可添加 `.vscode/launch.json` 用 `Cortex-Debug` 一键调试
- 调试前确保 `openocd` 可启动，且 DAPLink 可识别

### 0.8 典型问题快速排查
- 找不到 DAPLink：换线、换 USB 口、确认板子供电
- 能识别但无法下载：检查 `openocd` 配置文件（`interface/target`）和 SWD 连接
- `make` 失败：检查 CubeMX 生成的路径、启动文件和链接脚本是否完整
- 工程能编译但程序不跑：确认 `SystemClock_Config()`、GPIO 初始化和主循环逻辑

### 0.9 完成标准（通过即进入 FreeRTOS）
- [ ] 工程可 `Build` 通过
- [ ] 可稳定下载到板子
- [ ] 裸机 LED 闪烁正常

## 1. 项目概览
- 项目名：`STM32G474CEU7/V1.0.0`
- 目标：在 STM32G474CEU7 上完成 FreeRTOS 基础移植与常见外设任务开发
- 当前阶段：VSCode 开发环境搭建

## 2. 开发环境
- IDE：`VSCode`
- 工程生成：`STM32CubeMX`（Makefile）
- 工具链：`arm-none-eabi-gcc`
- 下载器：`DAPLink (CMSIS-DAP)`
- 下载/调试：`openocd + arm-none-eabi-gdb`
- HAL/LL 库版本：
- FreeRTOS 版本：

## 3. 学习路线（打勾推进）
- [ ] 裸机工程可稳定运行（GPIO/UART 定时输出）
- [ ] 加入 FreeRTOS 内核并成功启动调度器
- [ ] 创建 2~3 个基础任务（LED、串口、心跳）
- [ ] 验证任务优先级与时间片行为
- [ ] 使用队列（Queue）完成任务间通信
- [ ] 使用信号量（Semaphore）处理同步
- [ ] 使用软件定时器（Software Timer）
- [ ] 处理栈溢出与断言配置

## 4. 每日记录
### 日期：
### 今日目标：
### 实际完成：
### 遇到问题：
### 解决方法：
### 明日计划：

## 5. 关键知识点
### 5.1 任务（Task）
- 任务创建：
- 任务删除：
- 阻塞/挂起：
- 优先级设计经验：

### 5.2 调度（Scheduler）
- 抢占式/协作式：
- Tick 配置：
- 空闲任务（Idle Task）观察：

### 5.3 中断与 FreeRTOS
- 可在中断中调用的 API：
- `FromISR` 系列函数使用场景：
- 中断优先级分组与注意事项：

### 5.4 通信机制
- Queue：
- Semaphore（二值/计数）：
- Mutex：
- Event Group：

## 6. 常见问题排查清单
- [ ] `vTaskStartScheduler()` 后程序卡死
- [ ] SysTick 未正确接管
- [ ] 中断优先级设置不符合 FreeRTOS 要求
- [ ] 任务栈太小导致 HardFault
- [ ] 堆配置（heap_x.c）与内存不足

## 7. 调试记录（HardFault/异常）
- 现象：
- 触发条件：
- 调用栈信息：
- 根因分析：
- 修复方式：

## 8. 代码片段索引
- 任务创建示例：
- 队列收发示例：
- 信号量同步示例：
- 定时器回调示例：

## 9. 术语速记
- Tick：
- 上下文切换：
- 临界区：
- 优先级反转：

## 10. 里程碑
- M1：基础移植成功（日期：）
- M2：通信机制跑通（日期：）
- M3：稳定运行 24h（日期：）
