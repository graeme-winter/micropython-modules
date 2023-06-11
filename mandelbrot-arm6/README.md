# Mandelbrot Set Calculation

Ported from general ARM7 assembly, as an exercise in understanding. Key points are:

- must open file with `.thumb` to indicate which instruction set is in use
- using the right assembler - `arm-none-eabi-as` in this case
- slightly custom `Makefile` to facilitate this

Builds fine, allows assembly code to be called from µPython, [discussion here](https://graeme-winter.github.io/2023/04/2023-04-20.html).
