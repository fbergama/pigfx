@echo off

if "%1" == "" goto MISSINGTARGET
if "%1" == "dump" goto DUMP
if "%1" == "clean" goto CLEAN
set RPI=%1

if "%RPI%"=="4" goto SKIPUSPI
call makeuspi %RPI%
:SKIPUSPI

set OOB= asm.o exceptionstub.o synchronize.o mmu.o pigfx.o uart.o irq.o utils.o gpio.o mbox.o prop.o board.o actled.o framebuffer.o console.o gfx.o dma.o nmalloc.o uspios_wrapper.o ee_printf.o stupid_timer.o block.o emmc.o c_utils.o mbr.o fat.o config.o ini.o ps2.o keyboard.o binary_assets.o
set LIBUSPI= uspi/lib/libuspi.a

set CFLAGS= -Wall -Wextra -O2 -g -nostdlib -nostartfiles -fno-stack-limit -ffreestanding -fsigned-char
if "%RPI%"=="1" (
  set CFLAGS= %CFLAGS% -march=armv6j -mtune=arm1176jzf-s -DRPI=1
)
if "%RPI%"=="2" (
  set CFLAGS= %CFLAGS% -march=armv7-a -mtune=cortex-a7 -DRPI=2
)
if "%RPI%"=="3" (
  set CFLAGS= %CFLAGS% -march=armv8-a -mtune=cortex-a53 -DRPI=3
)
if "%RPI%"=="4" (
  set CFLAGS= %CFLAGS% -march=armv8-a -mtune=cortex-a53 -DRPI=4
  set LIBUSPI=
)

echo Compiling for Pi generation %RPI%

::Get Repository Version
md build 2>NUL
del build\tmp.txt 2>NUL
git describe --all --long > build\tmp.txt
set /p GIT_DESCRIBE=<build\tmp.txt
del build\tmp.txt
set BUILD_VERSION=%GIT_DESCRIBE:~-8%

:: Where's libgcc?
arm-none-eabi-gcc -print-libgcc-file-name -> build\tmp.txt
set /p LIBGCC=<build\tmp.txt
del build\tmp.txt

:: Build pigfx_config.h
setlocal DisableDelayedExpansion
set "search=$VERSION$"
set "replace=%BUILD_VERSION%"
del src\pigfx_config.h 2>NUL
for /F "delims=" %%a in (pigfx_config.h.in) DO (
  set line=%%a
  setlocal EnableDelayedExpansion
  >> src\pigfx_config.h echo(!line:%search%=%replace%!
  endlocal
)

:: Compiling c files
del build\*.* /Q 2>NUL
del *.elf /Q 2>NUL
:: Compiling s files
for %%s in (src/*.s) do arm-none-eabi-gcc src\%%s %CFLAGS% -c -o build\%%~ns.o
for %%c in (src/*.c) do arm-none-eabi-gcc src\%%c %CFLAGS% -c -o build\%%~nc.o
:: linking files
for %%A in (%OOB%) do (call :addbuild %%A)
arm-none-eabi-ld %OBJS% "%LIBGCC%" %LIBUSPI% -T memmap -o pigfx.elf
:: generate .img
arm-none-eabi-objcopy pigfx.elf -O binary pigfx.img

:: Copy img file
if "%RPI%"=="1" copy pigfx.img bin\kernel.img
if "%RPI%"=="2" copy pigfx.img bin\kernel7.img
if "%RPI%"=="3" copy pigfx.img bin\kernel8-32.img
if "%RPI%"=="4" copy pigfx.img bin\recovery7l.img

goto end

:: Add build\ before every o file
:addbuild
set OBJS= %OBJS% build\%1
goto END

:: Create dump file
:DUMP
arm-none-eabi-objdump --disassemble-zeroes -D pigfx.elf > pigfx.dump
goto end

:: Cleanup files
:CLEAN
del src\pigfx_config.h /Q 2>NUL
del build\*.* /Q 2>NUL
del *.hex /Q 2>NUL
del *.elf /Q 2>NUL
del *.img /Q 2>NUL
del *.dump /Q 2>NUL
del tags /Q 2>NUL
goto end

:MISSINGTARGET
echo Define Target, e.g. make 1 for Pi 1

:END
