/* ****************** check_inode_address.c  *****************
 * 通过文件索引号查找一个文件的节点的地址
 * 1. 一个地址需要16个字节
 * 2. 一些字是小端的，因此需要位移当需要的时候
 *
 * ***********************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define LOPATH "/dev/loop0"         // 文件位置
#define BUFFSIZE 1024               // 盘块大小
#define SUPER_BLOCK_ADDRESS 0x400   // 超级块开始地址
#define GROUP_DESC_ADDRESS 0x800    // 块组描述符起始位置
#define GROUP_DESC_SIZE 32	        // 一个块组描述符结构体的大小

int main(int argc, char *argv[]){
	if (argc != 2){
		fprintf(stderr, "usage: ./a.out [inode number]\n");
		return -1;
	}
	char buf[BUFFSIZE+1]={0};
	int inode_number = atoi(argv[1]);   // 将第二个参数转化为整数

	// inodes_per_group 和 inode_size 都在超级块里
	int inodes_per_group=0, inode_size=0;	
	int rd, fd = open(LOPATH, O_RDONLY);
	if (fd == -1){
		fprintf(stderr, "error: open file %s failed.\n", LOPATH);
		return -1;
	}
	
	// 寻找超级块中 inodes_per_group 的位置
	// inodes_per_group 是块中的第 11 个字
	lseek(fd,SUPER_BLOCK_ADDRESS+4*10,SEEK_SET);
	rd = read(fd,buf,BUFFSIZE);	

	//inodes_per_group 是小端 32 位
	for (int i = 0; i < 4; i++){
		inodes_per_group += (buf[i]&0xff) << (i*8);
	}
	printf("inodes per group: %d\n", inodes_per_group);

	// 定位超级块中的 inode_size
	lseek(fd,SUPER_BLOCK_ADDRESS+4*22,SEEK_SET);
	rd = read(fd,buf,2);
	
	// inode_size 是 16位（2字节）小端
	for (int i = 0 ; i < 2; i++){
		inode_size += (buf[i]&0xff) << (i*8);
	}
	printf("inode size: %d\n", inode_size);

	// 索引号起始于 1 而不是 0，因此需要减 1，这里得到块组号
	int group_number = (inode_number-1)/inodes_per_group;

	// 在索引表中的索引偏移是 (inode_number-1)%inodes_per_group
	int offset = (inode_number-1)%inodes_per_group;

	printf("group number: %d\n", group_number);
	printf("inode offset: %d\n", offset);
	
	// 根据根据块组号找到块组描述符结构体的位置，在 group descriptors 中
	int target_group_address = GROUP_DESC_ADDRESS + group_number*GROUP_DESC_SIZE;	

	printf("target group address: %08x\n", target_group_address);
	
	// 找到目标地址
	// 加 2*4 是因为 ext2_group_desc 的第三个字是 inode table 的首地址
	lseek(fd, target_group_address+2*4, SEEK_SET);
	
	// 获取索引表地址
	int inode_table_address = 0;
	if ((rd=read(fd, buf, 4)) == 4)
		for (int i = 0; i < 4; i++){
			inode_table_address += (buf[i]&0xff) << (i<<3);
		}
	printf("inode table address: %08x(block number)\n", inode_table_address);

	// 找到节点所指的文件数据的开始地址			
	printf("inode address: %08x\n", (inode_table_address<<10)+(offset*inode_size));

	return 0;
}
