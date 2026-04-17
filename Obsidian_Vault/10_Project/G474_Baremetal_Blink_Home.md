# G474_Baremetal_Blink 项目主页

## 核心入口

- 总教程：[Learn_Note.md](Learn_Note.md)
- 首页：[Home.md](../Home.md)

## 当前状态

- Build：OK
- Download/Debug：OK
- LED Blink：OK（`PC6`）
- UART 命令行：OK（`USART2`）
- Epaper 2.13 三色（WeAct）：OK（`SPI2 + PB12/PB14 + PA10/PA9`）
- 默认自动刷屏压测：ON（当前周期 `1000 ms`）

## 代码入口

- Main loop: [main.c](../../Core/Src/main.c)
- Interrupts: [stm32g4xx_it.c](../../Core/Src/stm32g4xx_it.c)
- Linker: [STM32G474XX_FLASH.ld](../../STM32G474XX_FLASH.ld)
- Startup: [startup_stm32g474xx.s](../../startup_stm32g474xx.s)
- Build: [Makefile](../../Makefile)

## 调试入口

- VSCode launch: [launch.json](../../.vscode/launch.json)
- VSCode tasks: [tasks.json](../../.vscode/tasks.json)
- VSCode keybindings: [keybindings.json](../../.vscode/keybindings.json)
- SVD: [STM32G474.svd](../../.vscode/cmsis-svd-stm32/stm32g4/STM32G474.svd)

## Epaper 常用命令（串口）

- `epdinit`
- `epdclear`
- `epdtest`
- `epdtext <msg>`
- `epdscroll [msg]`
- `epdautoon`
- `epdautooff`
- `epdsleep`
- `epdbusy`

## 下一步

- [x] 加入 UART 日志输出
- [x] 接入 2.13 三色墨水屏
- [x] 增加默认自动刷屏压测逻辑
- [ ] 记录 24h / 72h 压测结果（鬼影、坏点、BUSY 超时）
- [ ] 加入 FreeRTOS 内核
- [ ] 创建多任务并验证调度
