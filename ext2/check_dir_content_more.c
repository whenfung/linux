/* ****************** check_dir_content.c  **********************
 * 在 ext2 文件系统中，通过给定的索引节点地址得到一个目录在信息
 * 1. 一块地址占 16 字节
 * 2. 这些字都是小端，所以当读取它们的时候需要适当的转化
 * ************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFSIZE 1024            // 盘块大小
#define DIRECT 12                // 目录项大小
#define INDIRECT 256             
#define LOPATH "/dev/loop0"      // 磁盘分区路径

static const char* file_types[]={ 
	"unknown",
	"regular file",
	"directory",
	"character device",
	"block device",
	"fifo",
	"sock",
	"symlink",
	"max"	
};

void cat_dir_content(int fd, int i_block_number, int degree);

int main(int argc, char *argv[]){
	if (argc != 2){
		fprintf(stderr, "usage: ./a.out [inode address]\n");
		return -1;
	}
	char buf[BUFFSIZE] = {0};

	int i_blocks[16] = {0};
	int address = strtol(argv[1], NULL, 16);
	int fd = 0, i = 0, j = 0;
	
	if ((fd = open(LOPATH, O_RDONLY)) == -1){
		fprintf(stderr, "open file %s failed.\n", LOPATH);
		return -1;
	}
	
	// 找到开始地址
	if (lseek(fd,address,SEEK_SET) != -1){
		lseek(fd, 32+4*2, SEEK_CUR);	
		read(fd, buf, 64);
		for (i = 0; i < 15; i++){
			// 将地址放到 i_blocks 数组中
			for (j = 0; j < 4; j++){
				//i_blocks[i] += ((buf[i*4+j] & 0xff) << (j*4*2));
				i_blocks[i] += ((buf[(i<<2)+j] & 0xff) << (j<<3));
			}
		}
	}

	// 直接读取文件内容
	for (i = 0; i < 12; i++){
		if (i_blocks[i] == 0)break;
		cat_dir_content(fd, i_blocks[i], 0);
	}

	// 间接读取文件内容 (256)
	if (i_blocks[12] != 0){
		cat_dir_content(fd, i_blocks[12], 1);
	}

	// 间接读取文件内容 (64k)
	if (i_blocks[13] != 0){
		cat_dir_content(fd, i_blocks[13], 2);
	}

	// 间接读取文件内容(16m)
	if (i_blocks[14] != 0){
		cat_dir_content(fd, i_blocks[14], 3);
	}

	return 0;

}

// 使用递归形式，得到文件系统盘块中的目录内容
void cat_dir_content(int fd, int i_block_number, int degree){
	if (i_block_number == 0 || degree < 0) return;
	
	int count = 0;
	char buffer[BUFFSIZE+1]={0};

	// 一个盘块 = 1k = 1024 byte, 所以需要乘以 1024
	// 可以用 "i_block_number<<10" 方式做到
	lseek(fd, i_block_number<<10, SEEK_SET);
	count = read(fd, buffer, BUFFSIZE);

	if (count == 0){
		printf("read over:the end of the file.\n");
		return ;
	}
	else if (count == -1){
		fprintf(stderr, "read file failed.\n");
		return ;
	}

	// 当 degree = 0时, 解释目录内容
	if (degree == 0){
		int i = 0, j = 0, total_len = 0;
		char *name;
		
		while (1){
			int inode_number=0, rec_len=0, name_len=0, file_type=0;

			// 得到目录的索引号
			for (j = 0; j < 4; j++){
				inode_number += (buffer[i++]&0xff) << (j*8);
			}

			// 得到目录项长度，由于文件名是不定长的
			for (j = 0; j < 2; j++){
				rec_len += (buffer[i++]&0xff) << (j*8);
			}	
		
			if (inode_number==0||rec_len==0) break;
			else if (total_len >= 1024) break;
			else 	total_len+=rec_len;

			name_len = buffer[i++]&0xff;
			file_type = buffer[i++]&0xff;
	
			name = (char*)malloc(sizeof(char)*(name_len+1));
			memset(name, 0, sizeof(*name));

			for (j = 0; j < name_len; j++){
				name[j] = buffer[i++]&0xff;
			}
	
			// 记录下一个目录项的起始位置
			i = total_len;

			printf("name: %s\n", name);
			printf("name length: %d\n", name_len);
			printf("inode number: %d\n", inode_number);
			printf("file type: %s\n", file_types[file_type]);
			printf("directory entry length: %d\n\n", rec_len);
			free(name);
		}
	}
	// 继续递归，多级页表思想
	else {
		int address_buf[256] = {0};
		for (int i = 0; i < INDIRECT; i++){
			for (int j = 0; j < 4; j++){
				address_buf[i] += buffer[i*4+j] << (j*8);
			}
			// 进入递归
			cat_dir_content(fd, address_buf[i], (degree-1));
		}
		
	}
}
