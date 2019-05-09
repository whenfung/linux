#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<unistd.h>

int main(int argc,char *argv[]){
    int fd,len;
    char *ptr;
    if(argc<2){
        printf("please enter a file\n");    
        return 0;
    }   
    if((fd=open(argv[1],O_RDWR))<0){
        perror("open file error");
        return -1; 
    }   
    len = lseek(fd,0,SEEK_END);   
    ptr = (char*) mmap(NULL,len,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);  //读写得和open函数的标志相一致，否则会报错
    if(ptr == MAP_FAILED){
        perror("mmap error");
        close(fd);
        return -1; 
    }   
    close(fd);//关闭文件也 ok
    printf("length is %d\n",(int)strlen(ptr));
    printf("the %s content is:\n%s\n",argv[1],ptr);
    ptr[0]= getchar();//修改其中的一个内容
    printf("the %s content is:\n%s\n",argv[1],ptr);
    munmap(ptr,len);//将改变的文件写入内存
    return 0;
}
