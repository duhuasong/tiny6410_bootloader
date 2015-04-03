#include"stdio.h"
#include"lib/string.h"
#include"uart.h"
#include"command.h"
#include"main.h"
#include"nand.h"

static struct command commands[COMMAND_NUM] =
{
	{0,"help"},
	{1,"md"},
	{2,"mw"},
	{3,"nand"},
	{4,"bootm"},

};

void delay(volatile int i)
{
	while (i--);
}


int abort_boot(int bootdelay)
{
	int abort = 0;
	printf("Hit any key to stop autoboot:\t %d",bootdelay);
	
	while ((bootdelay > 0) && (!abort)) 
	{
		int i;
		--bootdelay;
		/* delay 100 * 10ms */
		for (i=0; !abort && i<500; ++i) 
		{
			if (UTRSTAT0& 0x1)/* we got a key press	check the uart Register*/
			{	
				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/

				(void) getc();  /* consume input	*/

				break;
			}
			delay (0x1000);
		}

		printf ("\b\b \b%d ", bootdelay);
	}
	printf ("\r\n");
	return abort;
}


static int getchar(void)
{
	char ichar;
	ichar = getc();
	if ((ichar == '\n') || (ichar == '\r'))//deal with Enter and newline 
	{
		putc('\n');
		ichar = '\n';
	}
	return ichar;
}


static  void getcommand(char *s)
{	
	char *p = s;
	while ((*p =getchar()) != '\n') 
	{
		if (*p != '\b')
			putc(*p++);
		else if(p > s)//deal with Backspace
			{
				putc('\b');				
				putc(' ');
				putc('\b');
				p--;
			}	
	}		
	*p = '\0';	
	putc('\r');
	//putc('\n');
}

int read_command_line()
{
	char console_buffer[MAX_CMDBUF_SIZE];		/* console I/O buffer	*/
	char *p ;
	int argc = 0;
	char *argv[CFG_MAXARGS +1];
	
	printf("%s",CFG_PROMPT);//show the command head
	getcommand(console_buffer);
	
	 p=strtok(console_buffer," ");
	 while(p!=NULL)
	  {
	        argv[argc]=p;
	        argc++;
	        p=strtok(NULL," ");         
	  }

	run_command(argc,argv);
	
	return argc;
}

void run_command(int argc, char * argv[])
{	
	int c=0;
	int i=0;

	for(c =0;c<COMMAND_NUM;c++)
	{
		if(strcmp(argv[0], commands[c].name)==0)
			break;	
	}

	switch(c)
	{
		case 0 :
			i = help(argc,argv);
			break;

		case 1:
			if(argc!=3)
			{
				printf("command Parameter is Error\r\n");
				printf("Try 'cp --help' for more information.\r\n");
				break;
			}
			i = md(argc,argv);
			break;

		case 2:
			if(argc!=4)
			{
				printf("command Parameter is Error\r\n");
				printf("Try 'cp --help' for more information.\r\n");
				break;
			}
			i = mw(argc,argv);
			break;

		case 3:
			if(argc!=5)
			{
				printf("command Parameter is Error\r\n");
				printf("Try 'cp --help' for more information.\r\n");
				break;
			}
			i = nand(argc,argv);
			break;
			
		case 4:
			i = bootm(argc,argv);
			break;
		case 5:
			read_command_line();
		//default:
			
	}

	if(i==1)
		printf("Do command %s successful!\r\n",argv[0]);
	else
		printf("Do command %s fail!\r\n",argv[0]);
	
	read_command_line();
	
}



int nand(int argc, char * argv[])
{	
	int offset;
	int adress;
	int page;

	if(strcmp(argv[1], "read")==0 );
	{
		adress = atoi(argv[2]); //adress of read to sdram
		offset = atoi(argv[3]); // nand adress	
		page = atoi(argv[4]) * 2048;   // 1 page = 2K 
		nand_read_page(offset,adress,page);
	}
	else if (strcmp(argv[1], "write")==0)
	{
		printf("nand write !\r\n");
	}
		
	return 1;
}


int md(int argc, char * argv[])
{
	volatile unsigned int *p = (volatile unsigned int *)0;
	int size,i,j;
	p = (volatile unsigned int *)atoi(argv[1]);//char to int
	size = atoi(argv[2]);

	for(i=0;i<size;)
	{
		printf("%x :     ",p+i);
		for(j=0;j<4;j++)
		{
			printf("%x  ",*(p+i));
			i++;
			if(i>=size)
				break;
		}
		printf("\r\n");
	}
	
	return 1;
}

int mw(int argc, char * argv[])
{
	volatile unsigned int *p = (volatile unsigned int *)0;
	int size,i;
	int value = 0;
	
	p = (volatile unsigned int *)atoi(argv[1]);//char to int
	size = atoi(argv[2]);
	value = atoi(argv[3]);
		
	for(i=0;i<size;i++)
	{
		*(p+i) = value;
	}
	
	return 1;
}

int bootm(int argc, char * argv[])
{
	boot_linux();
	return 1;
}

int help(int argc, char * argv[])
{
	printf("md            md [adress] [size] -- memory dispaly\r\n");
	printf("mw            md [adress] [size] [value]  --memory write\r\n");
	printf("nand read     nand read [aderss] [offset] [page] --nand read to memory\r\n");
	printf("nand write    nand read [aderss] [offset] [page] --nand read to memory\r\n");
	printf("bootm         boot for linux kernel\r\n");
	return 1;
}


int atoi(char * buf)
{
	unsigned int value = 0;
	int jinzhi = 10;
	int i =0;
	
	if(buf[0] == '0'&&buf[1] == 'x')
	{
		jinzhi = 16;
		i = 2;
	}

	while(buf[i])
	{
		int tmp;
		
		if (buf[i] <= '9' && buf[i] >= '0') 
			tmp = buf[i] - '0';
		else
			tmp = buf[i] - 'a' + 10;
					
		value = value * jinzhi + tmp;
		
		i++;
	}
	return value;
}










