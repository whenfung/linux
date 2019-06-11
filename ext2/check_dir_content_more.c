/* ****************** check_dir_content.c  **********************
 * check_dir_content.c is used to catch the information of a 
 * directory in the ext2 file system by the given inode address.
 * Note1 : one piece of address takes 16 bytes.	
 * Note2 : some words are little-endian, so they need to be shifted 
 *		   when read.
 * Email: 511138508@qq.com
 * ******************* By Victor Lin *****************************/
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

#define BUFFSIZE 1024
#define DIRECT 12
#define INDIRECT 256
#define LOPATH "/dev/loop0"

static char* file_types[]={
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
	char content_buf[BUFFSIZE+1] = {0};

	int i_blocks[16] = {0};
	int address_buf[256] = {0};
	int address = strtol(argv[1], NULL, 16);
	int line = 0, rdcount = 0;
	int fd = 0, i = 0, j = 0, k = 0, m = 0, n = 0;
	
	if ((fd = open(LOPATH, O_RDONLY)) == -1){
		fprintf(stderr, "open file %s failed.\n", LOPATH);
		return -1;
	}
	
	/*find the start adress*/
	if (lseek(fd,address,SEEK_SET) != -1){
		lseek(fd, 32+4*2, SEEK_CUR);	
		read(fd, buf, 64);
		for (i = 0; i < 15; i++){
		//put the address into the i_blocks array
			for (j = 0; j < 4; j++){
			//i_blocks[i] += ((buf[i*4+j] & 0xff) << (j*4*2));
			i_blocks[i] += ((buf[(i<<2)+j] & 0xff) << (j<<3));
			}
		}
	}

	/*read file content directly*/
	for (i = 0; i < 12; i++){
		if (i_blocks[i] == 0)break;
		cat_dir_content(fd, i_blocks[i], 0);
	}

	/*read file content indirectly(256)*/
	if (i_blocks[12] != 0){
		cat_dir_content(fd, i_blocks[12], 1);
	}

	/*read file content indirectly(64k)*/
	if (i_blocks[13] != 0){
		cat_dir_content(fd, i_blocks[13], 2);
	}

	/*read file content indirectly(16m)*/
	if (i_blocks[14] != 0){
		cat_dir_content(fd, i_blocks[14], 3);
	}

	return 0;

}

/* 
 * cat_dir_content is used for the system to catch the 
 * directory content from the blocks of the file system
 * using the recursion way. 
 */
void cat_dir_content(int fd, int i_block_number, int degree){
	if (i_block_number == 0 || degree < 0) return;
	
	int count = 0;
	char buffer[BUFFSIZE+1]={0};

	//one block = 1k = 1024 byte, so it needs to multiply 1024, 
	//which can be "i_block_number<<10" as well
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

	//when degree equal to 0, do explain the dir
	if (degree == 0){
		int i = 0, j = 0, total_len = 0;
		char *name;
		
		while (1){
		int inode_number=0, rec_len=0, name_len=0, file_type=0;
		//get the inode number of the directory
		for (j = 0; j < 4; j++){
			inode_number += (buffer[i++]&0xff) << (j*8);
		}

		//get the directory entry length
		for (j = 0; j < 2; j++){
			rec_len += (buffer[i++]&0xff) << (j*8);
		}
		
		if (inode_number==0||rec_len==0) break;
		else if (total_len >= 1024) break;
		else 	total_len+=rec_len;

		name_len = buffer[i++]&0xff;
		file_type = buffer[i++]&0xff;
	
		name = (char*)malloc(sizeof(char)*(name_len+1));
		memset(name, 0, sizeof(name));

		for (j = 0; j < name_len; j++){
			name[j] = buffer[i++]&0xff;
		}
	
		//record the index of next dir entry in the buffer indeed
		i = total_len;

		printf("name: %s\n", name);
		printf("name length: %d\n", name_len);
		printf("inode number: %d\n", inode_number);
		printf("file type: %s\n", file_types[file_type]);
		printf("directory entry length: %d\n\n", rec_len);
		free(name);
		}
	}
	//or it will do the function again to jump 
	//to the indirect address of the blocks
	else {
		int address_buf[256] = {0};
		for (int i = 0; i < INDIRECT; i++){
			for (int j = 0; j < 4; j++){
				address_buf[i] += buffer[i*4+j] << (j*8);
			}
			//do recursion
			cat_dir_content(fd, address_buf[i], (degree-1));
		}
		
	}
}
