/*
Group 22
Andrew Estes
Ahmad Joudeh
Viswa Rajagopalan
*/

#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
#include<stdlib.h>
#include<time.h>
#include<stdbool.h>
#include<ctype.h>

#define BLOCK_SIZE 1024
#define INODE_SIZE 64
#define DIR_SIZE 32

typedef struct {
    int isize;
    int fsize;
    int nfree;
    unsigned int free[200];
    char flock;
    char ilock;
    char fmod;
    unsigned int time;
    unsigned short ninode;
    unsigned short inode[100];
} superblock_type;

superblock_type superBlock;

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
dir_type root_dir[2];

int fd;

int open_fs(char *file_name){
    fd = open(file_name, O_RDWR | O_CREAT, 0600);

    if(fd == -1){
        return -1;
    }
    else{
        return fd;
    }
}

// Function to write inode
void inode_writer(int inum, inode_type inode){
   lseek(fd,2*BLOCK_SIZE+(inum-1)*INODE_SIZE,SEEK_SET); 
    write(fd,&inode,sizeof(inode));
}


int get_free_block(){
    if(superBlock.nfree == 0)
        return -1;
    return superBlock.free[superBlock.nfree--];
}

int add_free_block(unsigned int num){
    if(superBlock.nfree < 200){
        superBlock.free[superBlock.nfree++] = num;
        return 1;
    }
    return -1;
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
    root.addr[0] = 2 + superBlock.free[superBlock.nfree--];
    for (i=1;i<9;i++) root.addr[i]=-1;//all other addr elements are null so setto -1
    inode_writer(inum, root);
}

// initializes file system, superblock, root directory etc.
void initfs(char *file_name , int n1, int n2){
    int fd = open_fs(file_name);
    if(fd != -1){
        superBlock.isize = n2;
        superBlock.fsize = n1;
        if((n1 - n2 - 2) >= 200){ // in the case that there is enough blocks to fill free[]
            superBlock.nfree = 199;
            int i , j = 0;
            for(i=199; i > 0; i--){
                superBlock.free[i] = 2 + n2 + j;
                j++; // this for loop gives the next available
                // free block to the free array, 
                // from free[199] down to free[1]
            }
            //superBlock.free[0] = ;
        }
        else{
            superBlock.nfree = n1 - n2 - 2;
            int i , j = 0;
            for(i=superBlock.nfree; i > 0; i--){
                superBlock.free[i] = 2 + n2 + j;
                j++;
            }
        }
        
        // write superblock
        lseek(fd, BLOCK_SIZE, SEEK_SET);
        write(fd, &superBlock, 1);

        //init root dir
        root_dir[0].inode = 1;
        strcpy((char*) root_dir[0].filename, ".");
        root_dir[1].inode = 1;
        strcpy((char*) root_dir[1].filename, "..");

        //Root dir to disk
        lseek(fd, (n2+2) * BLOCK_SIZE, SEEK_SET);
        write(fd, &root_dir[0], DIR_SIZE);
        lseek(fd, (n2+2) * BLOCK_SIZE + DIR_SIZE, SEEK_CUR);
        write(fd, &root_dir[1], DIR_SIZE);
        
        inode_type inode1;
        fill_an_inode_and_write(1);
        inode1 = inode_reader(1,inode1);

        //write inodes
        lseek(fd, (2 * BLOCK_SIZE), SEEK_SET);
        for (int i = 0; i < superBlock.isize; i++ ) {
            fill_an_inode_and_write(i);
        } 

        lseek(fd, n1 * BLOCK_SIZE , SEEK_SET); // seeks to very end of file system, so that the correct data size is shown
        write(fd, " ", 1);
    }
    else
        printf("ERROR: File open failed.");
}

// quits program
void q() {
    printf("\nClosing file system and simulation...\n");
    sleep(1); // simulated runtime
    exit(0);
}

// runs user interaction for file system
void fs(){
    int n1;
    int n2;
    char str_n1[10000];
    char str_n2[10000];
    char filename[1000];

    printf("Please enter the desired name for the name containing your file system:\n\n>>");
    fgets(filename, 1000, stdin);
    filename[strlen(filename)-1] = '\0'; // remove newline and replace with null terminator

    printf("Please enter the maximum number of blocks for the filesystem (0 - 9999):\n\n>>");
    fgets(str_n1, 10000, stdin);
    n1 = atoi(str_n1);

    printf("Please enter the number of blocks dedicated to the i-nodes (0-9999):\n\n>>");
    fgets(str_n2, 10000, stdin);
    n2 = atoi(str_n2);

    printf("Initializing file system in %s with %d blocks and %d i-nodes...\n", filename, n1, n2);
    initfs(filename, n1, n2);
    sleep(2); // simulated runtime
    printf("File system initialized in file %s\n", filename);
    
    printf("\nPress Enter to return to the Main menu.\n\n>>");
    getchar();
}

// The main function
int main(){
    while(true){
        printf("Unix V6 File System Simulation\n");
        printf("==================================================================================================\n");
       
        // prompt user for input
        printf("Available commands:\n");
        printf("initfs - Initializes file system with specified parameters.\nq - Quits the simulation.\n\n>>");
        
        // initialize needed strings
        char input[1000];
        char command[1000];

        // accept input from user
        fgets(input, 1000, stdin);

        // convert input to lower case NOT FUNCTIONING PROPERLY CURRENTLY NOT BEING USED
        int i = 0;
        while(input[i]) {
            char ch = tolower(input[i]);
            i++;
            command[i] = ch;
        }

        // using if-else structure since switch statements in C cannot be used for strings.
        if (strcmp("initfs\n", input) == 0)
            fs();
        else if (strcmp("q\n", input) == 0)
            q();
        else{
            printf("\nERROR: invalid input\n\n\n");
            continue;
        }
    }
}
