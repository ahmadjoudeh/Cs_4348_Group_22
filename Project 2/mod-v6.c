#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
#include<stdlib.h>
#include<time.h>

#define BLOCK_SIZE 1024
#define INODE_SIZE 64

typedef struct {
    unsigned short flags;
    unsigned short nlinks;
    unsigned int uid;
    unsigned int gid;
    unsigned int size0;
    unsigned int size1;
    unsigned int addr[9];
    unsigned int actime;
unsigned int modtime;
} inode_type;

typedef struct { 
    unsigned int inode; 
    char filename[28];
} dir_type;  //32 Bytes long

inode_type root;

int fd;

int open_fs(char *file_name){
    fd = open(file_name, O_RDWR | O_CREAT, 0600);

    if(fd == -1){
        return -1;
    }
    else{
        return 1;
    }
}

// Function to write inode
void inode_writer(int inum, inode_type inode){

   lseek(fd,2*BLOCK_SIZE+(inum-1)*INODE_SIZE,SEEK_SET); 
    write(fd,&inode,sizeof(inode));
}

// Function to read inodes
inode_type inode_reader(int inum, inode_type inode){
   lseek(fd,2*BLOCK_SIZE+(inum-1)*INODE_SIZE,SEEK_SET); 
    read(fd, &inode, sizeof(inode));
    return inode;
}

// Function to write inode number after filling some fileds
void fill_an_inode_and_write(int inum){
inode_type root;
int i;

    root.flags |= 1 << 15; //Root is allocated
    root.flags |= 1 <<14; //It is a directory
    root.actime = time(NULL);
    root.modtime = time(NULL);

    root.size0 = 0;
    root.size1 = 2 * sizeof(dir_type);
    root.addr[0]=100; //assuming that blocks 2 to 99 are for i-nodes; 100 is the first data block that can hold root's directory contents
	for (i=1;i<9;i++) root.addr[i]=-1;//all other addr elements are null so setto -1
    inode_writer(inum, root);

}

// The main function
int main(){

    inode_type inode1;
    open_fs("Test_fs.txt");
    fill_an_inode_and_write(1);
    inode1=inode_reader(1,inode1);
    printf("Value of inode1's addr[0] is %d\n",inode1.addr[0]);
    printf("Value of inode1's addr[1] is %d\n",inode1.addr[1]);
}
