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
#include<stddef.h>

typedef struct TREASURE{
    int id;
    char user[35];
    float longitude,latitude;
    char clue[80];
    int value;
}TREASURE;

void read_line(char * buffer,size_t size)//functie generala de citire cu read
{
    ssize_t len=read(0,buffer,size-1);
    if(len>0)
    {
        if(buffer[len-1]=='\n')
            buffer[len-1]='\0';
        
    }
    else
    {
        buffer[0]='\0';
    }
}
TREASURE *creare_comoare()
{
    char temp[256];//un buffer temporar
    TREASURE *comoara=NULL;
    comoara=malloc(sizeof(TREASURE));
    if(comoara==NULL)
    {
        perror("Memory Not Enough For Creating New Treasure!\n");
        exit(-1);
    }
    printf("ID: ");
    fflush(stdout);
    read_line(temp,sizeof(temp));
    comoara->id=atoi(temp);

    printf("User: ");
    fflush(stdout);
    read_line(comoara->user,sizeof(comoara->user));

    printf("Longitude: ");
    fflush(stdout);
    read_line(temp,sizeof(temp));
    comoara->longitude=strtof(temp,NULL);//conversie string

    printf("Latitude: ");
    fflush(stdout);
    read_line(temp,sizeof(temp));
    comoara->latitude=strtof(temp,NULL);

    
    printf("Clue: ");
    fflush(stdout);
    read_line(comoara->clue,sizeof(comoara->clue));

    printf("Value: ");
    fflush(stdout);
    read_line(temp,sizeof(temp));
    comoara->value=atoi(temp);

    return comoara;

}

void loggin_action(const char *action,char *path)//functie care salveaza sau creaza(in cazul in care nu exista) in fisierul de log 
//data la care se intampla o actiune(adaugare,view etc) si stabileste si ce actiune se intampla
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

void list(char *hunt,char logg_path[1024])
{
    //verificam daca se poate deschide directorul
    DIR *dir=NULL;
    dir=opendir(hunt);

    if(dir==NULL)
    {
        perror("Hunt doesn't exist!");
        exit(-1);
    }    

//creeam pathul ex HUNT001/treasure_data
    char path[64];
    snprintf(path,sizeof(path),"%s/treasure_data",hunt);
//vedem daca putem deschide fisierul doar read-only
    int f=open(path,O_RDONLY,0777);
    if(f==-1)
    {
        perror("Failed at openning!");
        exit(-1);
    }
//folosim stat sa putem luam informatii despre director adica nume,nr bytes,ultima modificare 
    struct stat fis_date;
    if(stat(path,&fis_date)==-1)
    {
        perror("CAN'T GET INFO FROM FILE! ");
        close(f);
        closedir(dir);
        exit(-1);       
    }

    printf("Nume hunt: %s\n",hunt);
    printf("Dimensiune hunt: %ld\n",fis_date.st_size);
    printf("Last modification: %s\n",ctime(&fis_date.st_atim.tv_sec));//timpul exact al utlimei modificari

 
    printf("File content: \n\n");

    TREASURE *t_buf=malloc(sizeof(TREASURE));
    if(t_buf==NULL)
    {
        perror("NOT ENOUGH SPACE");
        free(t_buf);
        exit(-1);
    }
    int nr=1;

    while (read(f, t_buf, sizeof(TREASURE)))// cat timp pot citi o comoara din treasure_data se executa 
    {
        printf("Treasure %d\n", nr++);
        printf("ID: %d | User: %s | Longitude: %f | Latitude: %f | Clue: %s | Value: %d\n",
               t_buf->id, t_buf->user, t_buf->longitude, t_buf->latitude, t_buf->clue, t_buf->value);
    }

    char message[30];
//snprintf creeaza un sir formmatat intr-un buffer (adica intr-un string)
//e n=mai sigur pentru ca nu se poate intampla overflow
    snprintf(message,sizeof(message),"Listed %s",hunt);

    loggin_action(message,logg_path);

    free(t_buf);
    close(f);
    closedir(dir);
}


void remove_hunt(char *hunt,char logg_path[1024])
{
/*
    prima data deschid si verific daca pot deschide directorul
    dupa creez pathul catre fisierul treasure_data 
    si symlink 

    dupa incerc sa sterg pe rand fisierul treasure,dau remove la symlink,sterg fisierul log si in final directorul
*/


    DIR *dir=NULL;
    dir=opendir(hunt);
    if(dir==NULL)
    {
        perror("Doesn't exist :))");
        exit(-1);
    }

    char path[64];
    snprintf(path,sizeof(path),"%s/treasure_data",hunt);

    char symlink[64];
    snprintf(symlink,sizeof(symlink),"logged_hunt-%s",hunt);

    if(remove(path)==0)
    {
        printf("Treasure file deleted!\n");
    }
    else
    {
        printf("It didn't removed the file !\n");

    }

    if(unlink(symlink)==0)
    {
        printf("Symbolic link removed with succes!\n");
    }
    else{
        printf("Something happend and it didn't removed the symbolic link!\n");

    }
    if(remove(logg_path)==0)
    {
        printf("File loggin deleted!\n");
    }
    else{
        printf("Failed to delete loggin file !\n");
    }
    if(rmdir(hunt)==0)
    {
        printf("Hunt succesfully deleted!\n");
    }
    else{
        printf("Couldn't delete the HUNT!\n");
    }

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
        else if(strcmp(argv[1],"--list")==0)
        {
             list(argv[2],log_path);
        }
        else if(strcmp(argv[1],"--remove_hunt")==0)
        {
            remove_hunt(argv[2],log_path);
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