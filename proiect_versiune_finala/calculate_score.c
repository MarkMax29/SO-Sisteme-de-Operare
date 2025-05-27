#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct TREASURE{
    int id;
    char user[35];
    float longitudine,latitudine;
    char clue[80];
    int value;
}TREASURE;


int main(int argc,char **argv)
{
    if(argc!=2)
    {
        perror("not enough arg\n");
        exit(-1);
    }
    char *hunt=argv[1];

    //deschidem directorul hunt 
    struct stat st;
    if(stat(hunt,&st)==-1 || S_ISDIR(st.st_mode)==0)
    {
        perror("HUNT NOT FOUND\n");
        exit(-1);
    }

    char path[256];
    snprintf(path,sizeof(path),"%s/treasure_data",hunt);

    int fd=open(path,O_RDONLY);
    if(fd<0)
    {
        perror("Error at oppening treasure_data");
        exit(-1);
    }

    char users[100][35];
    int scores[100]={0},n=0;
    TREASURE t;

    while(read(fd,&t,sizeof(t))==sizeof(t))
    {
        int i;
        for(i=0;i<n;i++)
        {
            if(strcmp(users[i],t.user)==0)
                break;

        }
        if(i==n)
        {
            strncpy(users[n],t.user,sizeof(users[n])-1);
            users[n][sizeof(users[n])-1]='\0';
            scores[n]=t.value;
            n++;

        }
        else
        {
            scores[i]+=t.value;
        }

    }
    close(fd);

    printf("Scores for hunt %s:\n",hunt);
    for(int i=0;i<n;i++)
    {
        printf(" %s: %d\n",users[i],scores[i]);
    }


}
