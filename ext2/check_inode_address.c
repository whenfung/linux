/* ****************** check_inode_address.c  *****************
 * check_inode_address.c is used to find a file's inode address 
 * in the ext2 file system by the given inode number.
 * Note1 : one piece of address takes 16 bytes.	
 * Note2 : some words are little-endian, so they need to be 
		   shifted when read.
 * Email: 511138508@qq.com
 * ******************* By Victor Lin ************************/
#include <stdio.h>
#include <fcntl.h>

#define LOPATH "/dev/loop0"
#define BUFFSIZE 1024
#define SUPER_BLOCK_ADDRESS 0x400//the start address of super block
#define GROUP_DESC_ADDRESS 0x800 //the start address of group description
#define GROUP_DESC_SIZE 32	 //size(byte) of one group description struct

int main(int argc, char *argv[]){
	if (argc != 2){
		fprintf(stderr, "usage: ./a.out [inode number]\n");
		return -1;
	}
	char buf[BUFFSIZE+1]={0};
	int inode_number=atoi(argv[1]);
	//inodes per group and inode size are both in the super block
	int inodes_per_group=0, inode_size=0;	
	int rd, fd = open(LOPATH, O_RDONLY);
	if (fd == -1){
		fprintf(stderr, "error: open file %s failed.\n", LOPATH);
		return -1;
	}
	
	//seek to the position of inodes_per_group in super block
	//the inodes_per_group is 11th word of the block
	lseek(fd,SUPER_BLOCK_ADDRESS+4*10,SEEK_SET);
	rd = read(fd,buf,BUFFSIZE);	

	//inodes_per_group is 32 bit(4 bytes) little-endian
	for (int i = 0; i < 4; i++){
		inodes_per_group += (buf[i]&0xff) << (i*8);
	}
	//printf("inodes per group: %d\n", inodes_per_group);

	//seek to the position of inode_size in super block
	lseek(fd,SUPER_BLOCK_ADDRESS+4*22,SEEK_SET);
	rd = read(fd,buf,2);
	
	//inode_size is 16 bit(2 bytes) little-endian
	for (int i = 0 ; i < 2; i++){
		inode_size += (buf[i]&0xff) << (i*8);
	}
	//printf("inode size: %d\n", inode_size);

	//inode number starts with 1, so it needs to minus 1
	//group number = (inode number-1)/inodes per group
	int group_number = (inode_number-1)/inodes_per_group;

	//offset of the inode in inode table of the group = (inode number-1)%inodes per group
	int offset = (inode_number-1)%inodes_per_group;

	printf("group number: %d\n", group_number);
	printf("inode offset: %d\n", offset);
	
	//calculate the target address according to the inode number
	int target_group_address = GROUP_DESC_ADDRESS + group_number*GROUP_DESC_SIZE;	

	printf("target group address: %08x\n", target_group_address);
	
	//seek the address to the target address,
	//plus 2*4 because the offset of the attribute inode table address is 
	//the third word(one word = 4 bytes = 32 bit)
	//of the ext2_group_desc
	lseek(fd, target_group_address+2*4, SEEK_SET);
	
	//calculate the inode table address
	int inode_table_address = 0;
	if ((rd=read(fd, buf, 4)) == 4)
		for (int i = 0; i < 4; i++){
			inode_table_address += (buf[i]&0xff) << (i<<3);
		}
	printf("inode table address: %08x(block number)\n", inode_table_address);

	//seek to the inode table address
	//use "<<10" because the inode table address is block number indeed
	//which needs to multiply 1024 to be the real address in the device loop0
	lseek(fd, (inode_table_address<<10) + (offset*inode_size), SEEK_SET);
			
	printf("inode address: %08x\n", (inode_table_address<<10)+(offset*inode_size));

	return 0;
}
