#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include "inode_address.h"

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
void cat_file_content(int fd, int i_block_number, int degree);

int main(int argc, char *argv[]){
softlink_redirect:

	if (argc != 2){
		fprintf(stderr, "usage: ./a.out [aim]\n");
		return -1;
	}


	char query[BUFFSIZE] = {"ls -ila "};
	int itr = 0;
	while(argv[1][itr] != '\0'){
		query[itr+8] = argv[1][itr];
		itr++;
	}
	FILE *fp = popen(query,"r");
	if (fp == NULL)
	{
		fprintf(stderr, "open file %s failed.\n", argv[1]);
		return -1;
	}


	char ff[2]={0};
	char buf[BUFFSIZE] = {0};
	// char content_buf[BUFFSIZE+1] = {0};

	int i_blocks[16] = {0};
	// int address_buf[256] = {0};

	fread(buf,BUFFSIZE,1,fp);
	int inode_num = atoi(buf);
	
	char *p = strtok(buf," \r\n");
	itr = 0;
	if (p[0] == 't')
	{
		itr = 3;
	}
		p = strtok(NULL," \r\n");
	
	while(p){
		if (itr == 0)
		{
			itr++;
			if (p[0] == 'l')
			{
				itr = -1;
			}			
		}else if(itr == -1){
			argv[1] = p;
		}else if(itr >= 2){
			itr --;
			inode_num = atoi(p);
			
		}

		p = strtok(NULL," \r\n");
	}
	if (itr == -1)
	{
		pclose(fp);
		goto softlink_redirect;
	}
	if (inode_num == 0)
	{
		return -1;
	}


	int address = strtol(inode_addr(inode_num), NULL, 16);
	int line = 0, rdcount = 0;
	int fd = 0, i = 0, j = 0, m = 0, n = 0;
	if ((fd = open(LOPATH, O_RDONLY)) == -1){
		fprintf(stderr, "open file %s failed.\n", LOPATH);
		return -1;
	}	
	pclose(fp);
	if(lseek(fd,address,SEEK_SET) != -1){		
		read(fd,ff,2);
		printf("imode:%0x %0x\n", ff[1]&0xff,ff[0]&0xff);
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
		if ( ( ff[1]&0xff ) == 0x41 )
			cat_dir_content(fd,i_blocks[i],0);
		else
			cat_file_content(fd, i_blocks[i], 0);
	}

	/*read file content indirectly(256)*/
	if (i_blocks[12] != 0){
		if ( ( ff[1]&0xff ) == 0x41 )
			cat_dir_content(fd,i_blocks[12],0);
		else
			cat_file_content(fd, i_blocks[12], 1);
	}

	/*read file content indirectly(64k)*/
	if (i_blocks[13] != 0){
		if ( ( ff[1]&0xff ) == 0x41 )
			cat_dir_content(fd,i_blocks[13],0);
		else
			cat_file_content(fd, i_blocks[13], 2);
	}

	/*read file content indirectly(16m)*/
	if (i_blocks[14] != 0){
		if ( ( ff[1]&0xff ) == 0x41 )
			cat_dir_content(fd,i_blocks[14],0);
		else
			cat_file_content(fd, i_blocks[14], 3);
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
		int i,j; 
		for (i = 0; i < INDIRECT; i++){
			for ( j = 0; j < 4; j++){
				address_buf[i] += buffer[i*4+j] << (j*8);
			}
			//do recursion
			cat_dir_content(fd, address_buf[i], (degree-1));
		}
		
	}
}



/* 
 * cat_file_content is used for the system to catch the 
 * file content from the blocks of the file system
 * using the recursion way. 
 */
void cat_file_content(int fd, int i_block_number, int degree){
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

		//when degree equal to 0, do explain the file
		if (degree == 0){
			printf("%s",buffer);
		}
		//or it will do the function again to jump 
		//to the indirect address of the blocks
	else {
		int address_buf[256] = {0};
		int i,j; 
		for (i = 0; i < INDIRECT; i++){
			for ( j = 0; j < 4; j++){
				address_buf[i] += buffer[i*4+j] << (j*8);
			}
			//do recursion
			cat_file_content(fd, address_buf[i], (degree-1));
		}		
	}
}

