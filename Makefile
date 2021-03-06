CC      = arm-linux-gcc
LD      = arm-linux-ld
AR      = arm-linux-ar
OBJCOPY = arm-linux-objcopy
OBJDUMP = arm-linux-objdump


INCLUDEDIR 	:= $(shell pwd)/include                       #获得include目录的路径

CPPFLAGS   	:= -nostdinc -I$(INCLUDEDIR)                  #告诉编译器不要在标准系统目录中寻找头文件.只搜索`-I'选项指定的目录
                                                          #(以及当前目录,如果合适)
                                                          
export 	CC AR LD OBJCOPY OBJDUMP INCLUDEDIR CPPFLAGS     #导出变量，传递给子目录中的makefile

objs := start.o lowlevel_init.o sdram.o mynand.o main.o command.o uart.o clock.o lcd.o
#		  drivers/nand/libnand.a cpu/libs3c64xx.a

boot.bin: $(objs)
	${LD} -T boot.lds -o boot.elf $^
	${OBJCOPY} -O binary -S boot.elf $@
	${OBJDUMP} -D boot.elf > boot.dis

#进入drivers/nand/中去编译
#
#.PHONY : drivers/nand/libnand.a                         
#drivers/nand/libnand.a:
#	cd drivers/nand; make ; cd ../..
	
#.PHONY : cpu/libs3c64xx.a
#cpu/libs3c64xx.a:
#	cd cpu/; make ; cd ../
		
		
		
		
%.o:%.c
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%.o:%.S
	${CC} $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

.PHONY : clean
clean:
	rm -f *.bin *.elf *.dis *.o
	




