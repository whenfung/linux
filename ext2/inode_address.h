#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#define LOPATH "/dev/loop0"         // 磁盘分区位置
#define BUFFSIZE 1024               // 盘块大小
#define SUPER_BLOCK_ADDRESS 0x400   // 超级块起始位置
#define GROUP_DESC_ADDRESS 0x800    // 块组描述符起始位置
#define GROUP_DESC_SIZE 32	        // 一个块组描述符结构体的大小

char* inode_addr(int inode_number){
	
	int i;
	char buf[BUFFSIZE+1]={0};

	// inodes_per_group 和 inode_size 都在超级块中
	int inodes_per_group=0, inode_size=0;	
	int rd, fd = open(LOPATH, O_RDONLY);
	if (fd == -1){
		fprintf(stderr, "error: open file %s failed.\n", LOPATH);
		return 0;
	}
	
	// inodes_per_group 在超级块中的第 11 个字
	lseek(fd,SUPER_BLOCK_ADDRESS+4*10,SEEK_SET);
	rd = read(fd,buf,BUFFSIZE);	

	// inodes_per_group is 32 bit(4 bytes) little-endian
	for (i = 0; i < 4; i++){
		inodes_per_group += (buf[i]&0xff) << (i*8);
	}
	// printf("inodes per group: %d\n", inodes_per_group);

	// 读取超级块中的 inode_size 
	lseek(fd,SUPER_BLOCK_ADDRESS+4*22,SEEK_SET);
	rd = read(fd,buf,2);
	
	//inode_size is 16 bit(2 bytes) little-endian
	for (i = 0 ; i < 2; i++){
		inode_size += (buf[i]&0xff) << (i*8);
	}
	//printf("inode size: %d\n", inode_size);

	// inode_number 从 1 开始, 因此计算时需要减 1（磁盘内部从 0 开始）
	// group_number = (inode_number-1)/inodes_per_group
	int group_number = (inode_number-1)/inodes_per_group;

	// 节点在索引表内的偏移 = (inode_number-1)%inodes_per_group
	int offset = (inode_number-1)%inodes_per_group;

	printf("group number: %d\n", group_number);
	printf("inode offset: %d\n", offset);
	
	// 根据索引节点计算目标地址
	int target_group_address = GROUP_DESC_ADDRESS + group_number*GROUP_DESC_SIZE;	

	printf("target group address: %08x\n", target_group_address);
	
	// 搜索目标地址
	// 加 2*4 是因为 the offset of the attribute inode table address is 
	// the third word(one word = 4 bytes = 32 bit)
	// of the ext2_group_desc
	lseek(fd, target_group_address+2*4, SEEK_SET);
	
	// 计算索引表的位置
	int inode_table_address = 0;
	if ((rd=read(fd, buf, 4)) == 4)
		for (i = 0; i < 4; i++){
			inode_table_address += (buf[i]&0xff) << (i<<3);
		}
	printf("inode table address: %08x(block number)\n", inode_table_address);

	// 搜索索引表的地址
	// 使用 "<<10" 是因为索引表记录的是盘块号
	// 需要乘 10 得到 loop0 中真正的偏移地址
	lseek(fd, (inode_table_address<<10) + (offset*inode_size), SEEK_SET);
			
	printf("inode address: %08x\n", (inode_table_address<<10)+(offset*inode_size));

	char* buffer = (char*)malloc(9*sizeof(char));
	sprintf(buffer,"%08x", (inode_table_address<<10)+(offset*inode_size));

	return buffer;
}

