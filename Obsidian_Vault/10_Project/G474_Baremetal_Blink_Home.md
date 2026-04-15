# G474_Baremetal_Blink 项目主页

## 核心入口

- 总教程：[Learn_Note.md](Learn_Note.md)
- 首页：[Home.md](../Home.md)

## 当前状态

- Build：OK
- Download/Debug：OK
- LED Blink：OK（`PC6`）

## 代码入口

- Main loop: [main.c](../../Core/Src/main.c)
- Interrupts: [stm32g4xx_it.c](../../Core/Src/stm32g4xx_it.c)
- Linker: [STM32G474XX_FLASH.ld](../../STM32G474XX_FLASH.ld)
- Startup: [startup_stm32g474xx.s](../../startup_stm32g474xx.s)
- Build: [Makefile](../../Makefile)

## 调试入口

- VSCode launch: [launch.json](../../.vscode/launch.json)
- VSCode tasks: [tasks.json](../../.vscode/tasks.json)
- SVD: [STM32G474.svd](../../.vscode/cmsis-svd-stm32/stm32g4/STM32G474.svd)

## 下一步

- [ ] 加入 UART 日志输出
- [ ] 加入 FreeRTOS 内核
- [ ] 创建多任务并验证调度
