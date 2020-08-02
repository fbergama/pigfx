@echo off

if "%1" == "" goto MISSINGTARGET

set RPI=%1

set OOBUSPI= uspilibrary.o dwhcidevice.o dwhciregister.o dwhcixferstagedata.o usbconfigparser.o usbdevice.o usbdevicefactory.o usbendpoint.o usbrequest.o usbstandardhub.o devicenameservice.o macaddress.o usbfunction.o smsc951x.o lan7800.o string.o util.o usbmassdevice.o dwhciframeschednper.o dwhciframeschedper.o keymap.o usbkeyboard.o dwhcirootport.o usbmouse.o dwhciframeschednsplit.o usbgamepad.o synchronize.o usbstring.o usbmidi.o

set CFLAGS= -Wall -Wno-psabi -fsigned-char -fno-builtin -nostdinc -nostdlib -std=gnu99 -undef -mno-unaligned-access -DRASPPI=%RPI% -I ../include -O2
if "%RPI%"=="1" (
  set CFLAGS= %CFLAGS% -march=armv6j -mtune=arm1176jzf-s
)
if "%RPI%"=="2" (
  set CFLAGS= %CFLAGS% -march=armv7-a -mtune=cortex-a7
)
if "%RPI%"=="3" (
  set CFLAGS= %CFLAGS% -march=armv8-a -mtune=cortex-a53
)

echo Compiling USPI for Pi generation %RPI%
cd uspi/lib

:: Compiling c files
del *.o  2>NUL
del libuspi.a 2>NUL
for %%c in (*.c) do arm-none-eabi-gcc %%c -c %CFLAGS%
for %%A in (%OOBUSPI%) do arm-none-eabi-ar cr libuspi.a %%A

cd..
cd..
goto end

:MISSINGTARGET
echo Define Target, e.g. make 1 for Pi 1

:END