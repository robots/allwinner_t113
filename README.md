# Bla

This is Freertos on Allwinner T113 chip. (chip because its not mcu more like SOC)


## features of this project

- FreeRTOS with single core (yes i suck, no smp yet)
- interrupt controller GICv2
- OHCI using tinyUSB
- Working graphics with 800x480 lcd (chineese noname/nomarking lcd)
- twi is not working/finished
- uart output is blocking (yes seriously no dma or interrupts)

### Some details

Clocking of the cpu is 1008MHz.

DDR3 is initialized by the proprietary code in xfel. There is no bootloader yet. Get uboot or something.

USB OHCI Support is a bit hackish. I had to add cache invalidate/clear on some places. It will need very minor change for clean implementation.

LCD output is single buffered. Double buffer is written already, but not tested or enabled yet.

### Documentation

- https://github.com/mangopi-sbc/MQ.git
- https://github.com/mangopi-sbc/aw-doc.git
- https://github.com/Tina-Linux/d1s-melis.git


### License

There are many projects included here: TinyUSB (MIT License), tinyprintf(BSD3), TLSF(BSD), FreeROTS(MIT License), CMSIS_5(Apache license v2.0)

Startup (start.S + parts of irq.c and asm/*S) are from Xboot project

Other than that all files are my work, and released under public domain. Do whatever you want :-)