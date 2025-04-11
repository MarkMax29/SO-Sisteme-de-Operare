#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>//pentru optiunile de deschidere a fisierului
#include<unistd.h>
#include<sys/stat.h>//asta e pentru mkdir 
#include<sys/types.h>//pt director si mkdir 
#include<dirent.h>//biblioteci pentru directoare 
#include<time.h>
#include<errno.h>
#include<unistd.h>


typedef struct TREASURE{
    int id;
    char user[35];
    float longitude,latitude;
    char clue[80];
    int value;
}TREASURE;

TREASURE *creare_comoare()
{
    TREASURE *comoara=NULL;
    comoara=malloc(sizeof(TREASURE));
    if(comoara==NULL)
    {
        perror("Memory Not Enough For Creating New Treasure!\n");
        exit(-1);
    }
    printf("ID: ");
    scanf("%d",&comoara->id);

    getchar();
    printf("User: ");
    fgets(comoara->user,34,stdin);
    comoara->user[strlen(comoara->user)-1]='\0';

    printf("Longitude: ");
    scanf("%f",&comoara->longitude);

    printf("Latitude: ");
    scanf("%f",&comoara->latitude);

    getchar();
    printf("Clue: ");
    fgets(comoara->clue,79,stdin);
    comoara->clue[strlen(comoara->clue)-1]='\0';

    printf("Value: ");
    scanf("%d",&comoara->value);

    return comoara;

}

void loggin_action(const char *action,char *path)
{
    int fis_log=open(path, O_CREAT|O_APPEND|O_WRONLY,0644);//USER rw, group & others doar r e de preferat la log 
    if(fis_log==-1)
    {
        perror("FAILED TO OPEN THE LOG FILE !");
        exit(-1);
    }

    time_t now=time(NULL);//obtine timpul actual //
    struct tm *t=localtime(&now);
    char timestamp[100];
    strftime(timestamp,sizeof(timestamp),"[%Y-%m-%d %H:%M:%S]",t); //practic transforma timestamp-ul intr-un string 

    char log[256];
    snprintf(log,sizeof(log),"USED ACTION: %s at %s\n",action,timestamp);

    if(write(fis_log,log,strlen(log))==-1)
    {
        perror("Error at writing into log file !\n");
        exit(-1);
    }

    close(fis_log);



}


void add_treasure(char *hunt,char logg_path[1024])
{
    DIR *dir=NULL;
    if((dir=opendir(hunt))==NULL)// verificam daca hunt-ul nu exista deja si incercam sa il cream
    {
        if(mkdir(hunt,0755)==0)// permisiuni user: rwx, group:rx, others:rx
        {// ar mai fi posbil 0777 dar e securitate slaba pentru proiecte
            printf("Directory %s created with succes!\n",hunt);
        }
        else
        {
            perror("ERROR AT CREATING NEW DIRACTORY!\n");
            exit(-1);
        }
    }

    char path[64];
    snprintf(path,sizeof(path),"%s/treasure_data",hunt);

    int f=open(path,O_CREAT|O_WRONLY|O_APPEND, 0777);

    if(f==-1)
    {
        perror("FAILED TO OPEN FILE WITH TREASURE !");
        exit(-1);
    }

    TREASURE *comoara=creare_comoare();


    if(write(f,comoara,sizeof(TREASURE))==-1)
    {
        perror("CAN'T ADD A NEW TREASURE");
        free(comoara);
        close(f);
        exit(-1);
    }
    else
    {
        printf("NEW TREASURE ADDED !\n");

    }

 char slink[128];
 snprintf(slink,sizeof(slink),"logged_hunt-%s",hunt);

 struct stat s;

    if(lstat(slink,&s)==-1)
    {
        if(errno==ENOENT)
        {   
            if(symlink(logg_path,slink)==-1)
            {
                perror("ERROR CREATING SYMBOLIC LINK !\n");
                exit(-1);
            }
        
        else
            {
             printf("Symbolik link succeded !\n");
            }
        }
    }

    loggin_action("Added Treasure",logg_path);

    free(comoara);
    close(f);
    closedir(dir);

    
}
int main(int argc, char ** argv)
{
    
    char log_path[1024];
    snprintf(log_path, sizeof(log_path), "%s/logged_hunt.txt", argv[2] ); // se creaza path ul catre log 

    if(argc==3)
    {
        if(strcmp(argv[1],"--add")==0)
            {
                add_treasure(argv[2],log_path);
            }
        else if(strcmp(argv[1],"--list"))
        {
               // list(...)   
        }
        else if(strcmp(argv[1],"--remove_hunt"))
        {
            //remove_hunt(...)
        }
        else
            {
                perror("NO ARGUMENTS FOUND!");
                exit(-1);
            }
    }
    else if(argc==4)
    {
        if(strcmp(argv[1],"--view"))
        {
            //int hunt_id=atoi(argv[2]);
            int id=atoi(argv[3]);
            //view(...)
        }
        else if(strcmp(argv[1],"--remove_treasure"))
        {
          //  int hunt_id=atoi(argv[2]);
            int id=atoi(argv[3]);
            //remove_treasure(...)
        }
        else{
            perror("NOT GOOD ARGUMENTS!");
            exit(-1);
        }
    }
    else if(argc<3)
    {
        perror("NOT ENOUGH ARGUMENTS !");
        exit(-1);
    }
    return 0;
}