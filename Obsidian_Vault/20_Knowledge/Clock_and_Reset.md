# Clock and Reset Notes

## Clock source basics
- `HSI`: internal high-speed clock, easiest for bring-up.
- `HSE`: external crystal/clock source, better accuracy.
- `LSE`: 32.768 kHz external crystal, mainly for RTC.
- `HSI48`: internal 48 MHz source, often used by USB.

## Practical rule
- First bring-up: prefer simple/stable config.
- If boot hangs in clock init, check crystal settings first.

## Reset signal notes
- Frequent `external reset detected` in OpenOCD means NRST line is toggling.
- Check reset circuit, cable quality, and power stability.

