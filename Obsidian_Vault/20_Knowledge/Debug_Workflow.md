# Debug Workflow (VSCode + DAPLink)

## Build

```bash
make -j8
```

## Flash and run

```bash
openocd -f interface/cmsis-dap.cfg -f target/stm32g4x.cfg \
  -c "adapter speed 1000" \
  -c "program build/G474_Baremetal_Blink.elf verify reset exit"
```

## Common issue

- Symptom: `OpenOCD: GDB Server Quit Unexpectedly`
- Cause: existing `openocd` is already running.
- Fix:

```bash
pkill -f openocd
pkill -f arm-none-eabi-gdb
```
