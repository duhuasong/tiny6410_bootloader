led.bin: start.o main.o clock.o uart.o sdram.o
	arm-linux-ld -Ttext 0x50000000 -o led.elf $^
	arm-linux-objcopy -O binary led.elf led.bin
	arm-linux-objdump -D led.elf > led_elf.dis
%.o : %.S
	arm-linux-gcc -o $@ $< -c

%.o : %.c
	arm-linux-gcc -o $@ $< -c 

clean:
	rm  *.o led.elf led_elf.dis *.bin
