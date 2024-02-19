PROGRAMMER_DIR = /home/fran/.local/stm32/STM32CubeProgrammer/bin/
PROGRAMMER = $(PROGRAMMER_DIR)STM32_Programmer_CLI

TARGET = Debug/horno.elf

all: Debug/makefile
	make -C Debug -j4 all

flash: all
	 $(PROGRAMMER) -c port=SWD mode=UR reset=HWrst -w $(TARGET) --verify

debug: flash
	ST-LINK_gdbserver -p 61234 -l 1 -d -z 61235 -s -cp $(PROGRAMMER_DIR) -m 0 -k --halt &
	arm-none-eabi-gdb $(TARGET) -ex "target remote localhost:61234"

clean:
	make -C Debug -j4 clean
