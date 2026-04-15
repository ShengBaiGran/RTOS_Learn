# G474_Baremetal_Blink

## Status
- Build: OK
- Download/Debug: OK (`DAPLink + OpenOCD + Cortex-Debug`)
- LED blink: OK

## Code Entry
- Main loop: [main.c](../../Core/Src/main.c)
- Interrupts: [stm32g4xx_it.c](../../Core/Src/stm32g4xx_it.c)
- Linker script: [STM32G474XX_FLASH.ld](../../STM32G474XX_FLASH.ld)
- Startup: [startup_stm32g474xx.s](../../startup_stm32g474xx.s)
- Build file: [Makefile](../../Makefile)

## Debug Entry
- VSCode launch: [launch.json](../../.vscode/launch.json)
- VSCode tasks: [tasks.json](../../.vscode/tasks.json)
- SVD: [STM32G474.svd](../../.vscode/cmsis-svd-stm32/stm32g4/STM32G474.svd)

## Next Milestones
- [ ] Add UART log print
- [ ] Add FreeRTOS kernel
- [ ] Create 2 tasks and verify scheduling

