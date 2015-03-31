#define NFCONF          (*((volatile unsigned long *)0x70200000))
#define NFCONT          (*((volatile unsigned long *)0x70200004))
#define NFCMMD          (*((volatile unsigned long *)0x70200008))
#define NFADDR          (*((volatile unsigned long *)0x7020000C))
#define NFDATA          (*((volatile unsigned char *)0x70200010))
#define NFSTAT          (*((volatile unsigned long *)0x70200028))

//#define PAGESIZE (8*1024)   // K9GAG08U0E Flash 1Page = (8K + 436)Bytes
#define PAGESIZE 2048               // K9K8G08U0A Flash 1Page = (8K + 64)Bytes

#define NAND_ENABLE_SELECT()	(NFCONT &= ~(1 << 1))
#define NAND_DISABLE_SELECT()	(NFCONT |= (1 << 1))

void nand_send_cmd(unsigned char cmd)
{
	NFCMMD = cmd;
}

void nand_wait_ready(void)
{
	while ((NFSTAT & 0x1) == 0);
}


static void nand_reset(void)
{
	/* 选中 */
	NAND_ENABLE_SELECT();
	
	/* 发出0xff命令 复位命令*/
	nand_send_cmd(0xff);

	/* 等待就绪 */
	nand_wait_ready();
	
	/* 取消选中 */
	NAND_DISABLE_SELECT();
}

/*
void nand_send_addr(unsigned int addr)
{		
	unsigned int page   = addr / PAGESIZE;
	unsigned int colunm = addr % PAGESIZE;
	// 这两个地址表示从页内哪里开始 
	NFADDR = (colunm & 0xff);
	nand_wait_ready();
	NFADDR = ((colunm >> 8) & 0x3f);
	nand_wait_ready();
	// 下面三个地址表示哪一页 
	NFADDR = (page & 0xff);
	nand_wait_ready();
	NFADDR = ((page >> 8) & 0xff);
	nand_wait_ready();
	NFADDR = ((page >> 16) & 0xf);
	nand_wait_ready();

}
*/
void nand_send_addr(unsigned int addr)
{		
	unsigned int page   = addr / PAGESIZE;
	unsigned int colunm = addr % PAGESIZE;
	/* 这两个地址表示从页内哪里开始 */
	NFADDR = (colunm & 0xff);
	nand_wait_ready();
	NFADDR = ((colunm >> 8) & 0xf);
	nand_wait_ready();
	/* 下面三个地址表示哪一页 */
	NFADDR = (page & 0xff);
	nand_wait_ready();
	NFADDR = ((page >> 8) & 0xff);
	nand_wait_ready();
	NFADDR = ((page >> 16) & 0x7);
	nand_wait_ready();

}

void nand_init(void)
{
	#define TACLS    2
	#define TWRPH0    7
	#define TWRPH1    7
	/* 设置时间参数:HCLK = 7.5ns */
	NFCONF &= ~((1<<30) | (7<<12) | (7<<8) | (7<<4)); // clear 0 first
	NFCONF |= ( (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4) ); //set valve second
	/* 使能nand flash controller */
	NFCONT |= (3<<0); 
	
	nand_reset();//nand reset
}


void nand_read_page(unsigned int nand_start, unsigned int ddr_start, unsigned int len)
{
	//由于S3C6410的Nand Flash控制器每次只能读8个数据所以ddr_start只能强制类型转换为char
	unsigned char *buf = (unsigned char *)ddr_start;
	int colunm = nand_start % PAGESIZE;
	unsigned int i = 0;
	/* 1. 选中 */
	NAND_ENABLE_SELECT();

	while (i < len)
	{
		/* 2. 发出读命令00h */
		nand_send_cmd(0x00);

		/* 3. 发出地址(分5步发出) */
		nand_send_addr(nand_start);
		
		/* 4. 发出读命令30h */
		nand_send_cmd(0x30);

		/* 5. 判断状态 */
		nand_wait_ready();

		/* 6. 读数据 */
		for (; (colunm < PAGESIZE) && (i < len); colunm++)
		{
			buf[i] = NFDATA;
			i++;
			
		}
		nand_start = nand_start+PAGESIZE;
		colunm = 0;
	}

	/* 7. 取消选中 */		
	NAND_DISABLE_SELECT();
}

/*
void nand_read_id(unsigned char *buf)
{
	int i;
	
	NAND_ENABLE_SELECT();

	nand_send_cmd(0x90);

	nand_send_addr(0x0);

	for (i = 0;i<100;i++); 

	for (i=0;i<6;i++)	
		*(buf+i) = NFDATA;

	NAND_DISABLE_SELECT();	
}
*/
void nand_read_id(unsigned char *buf)
{
	int i;
	
	NAND_ENABLE_SELECT();

	nand_send_cmd(0x90);

	nand_send_addr(0x0);

	for (i = 0;i<100;i++); 

	for (i=0;i<5;i++)	
		*(buf+i) = NFDATA;

	NAND_DISABLE_SELECT();	
}



int nand_boot_copy_to_ddr(unsigned int nand_start, unsigned int ddr_start, unsigned int len)
{
	/*
	 *S3C6410启动时拷贝的8K代码不是存储在Nand flash的第一页上，
	 *而是存储在Nand flash的前4页上，每页2K，总共8K，
	 *这是S3C6410芯片的硬件结构决定的！所以启动代码要单独复制到DRAN中去
	 */

	unsigned char *buf = (unsigned char *)ddr_start;
	int col ;//colunm
	int i = 0;
	len = len/2048+1; //page 
	/* 1. 选中 */
	NAND_ENABLE_SELECT();
	
	while (i < len)
	{
		/* 2. 发出读命令00h */
		nand_send_cmd(0x00);

		/* 3. 发出地址(分5步发出) */
		nand_send_addr(nand_start);
		/* 4. 发出读命令30h */
		nand_send_cmd(0x30);

		/* 5. 判断状态 */
		nand_wait_ready();

		/* 6. 读数据 */
		for (col = 0; col < 2048; col++)
		{
			buf[col] =  NFDATA;	
		}
		buf = buf+2048;
		nand_start = nand_start + PAGESIZE;
		i++;//page ++	
	}

	/* 7. 取消选中 */		
	NAND_DISABLE_SELECT();
	return 0;
}


















