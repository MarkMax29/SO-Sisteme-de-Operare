#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>//pentru fork(),getpid()
#include <signal.h>//pentru semnale SIGUSR1, SIGTERM, kill()
#include <sys/types.h> //ca sa pot declara pid_t
#include <fcntl.h>//pentri open()
#include <string.h>
#include <sys/wait.h> //waitpad()
#include <dirent.h>
#include <sys/stat.h>

#define COM_size 111
static pid_t monitor_pid=-1; //acesta e PID monitorului si cand nu exista atunci e -1
static int monitor_running=0; // un flag ce zice ca monitoru ruleaza sau nu 

int pfd_list_hunts[2];
int pfd_list_treasure[2];
int pfd_view[2];
int pfd_calc[2];

void write_in_pipe(int pfd[2])
{
    close(pfd[0]);//inchidem capatul de citire al pipe-ului
    dup2(pfd[1],STDOUT_FILENO);//redirectionam stdout catre pipe
    close(pfd[1]);//inchidem capatul de scriere al pipe-ului
}

void read_from_pipe(int fd)
{
    char buf[256];
    ssize_t n;
    while((n=read(fd,buf,sizeof(buf)))>0)
    {
        write(STDOUT_FILENO,buf,n);//scriem in stdout
    }
    close(fd);
}
void write_command(const char *message)//functie care transmite comanda de la hub la monitor
{   
    //contruiesc calea fisierului cu snprintf,deoarece acesta garanteaza ca by scrie mai mult decat 
    //dimensiunea bufferului
    char path[50];
    snprintf(path,sizeof(path),"command_file");

    int f=open(path,O_CREAT|O_WRONLY|O_TRUNC,0777);
    if(f<0)
    {
        perror("Error at oppening command_file\n");
        exit(-1);
    }

    if(write(f,message,strlen(message))<0)
    {
        perror("Error at writing to command_file\n");
        close(f);
        exit(-1);
    }
    close(f);
}


void list_all_hunts() {
    DIR *dir = opendir("."); //deschide directorul curent(folderul din care rulezi programul)
    if (!dir) {
        perror("Nu s-a putut deschide directorul curent\n");
        return;
    }

    struct dirent *entry;

    //parcurgem fiecare intrare (fisier sau director)
    while ((entry = readdir(dir)) != NULL) {
        //ignorăm "." și ".." și fișierele care nu sunt directoare 
        //"."= directorul curent, iar ".." este directorul părinte
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)// apar ca shortcuturi ,sistemul de operare le include by default
    {
    continue;
    }
    struct stat st;
    if (stat(entry->d_name, &st) != 0 || !S_ISDIR(st.st_mode)) //daca nu e director trecem peste 
        {
            continue;
        }
        // formăm calea către fișierul treasure_data din acel director
        char path[512];
        snprintf(path, sizeof(path), "%s/treasure_data", entry->d_name);

        //deschidem fișierul
        int fd = open(path, O_RDONLY);
        if (fd < 0)
            continue; //dacă nu există,trecem mai departe

        int count = 0;
        //structură locală pentru a citi datele unei comori
        struct {
            int id;
            char user[35];
            float longitude, latitude;
            char clue[80];
            int value;
        } treasure;

    
        while (read(fd, &treasure, sizeof(treasure)) > 0)
            count++;

        close(fd); 

        printf("%s: %d treasures\n", entry->d_name, count);
    }

    closedir(dir); 
}

void handle_sigusr1(int sig_type)
{
    (void)sig_type;//face ca sa nu imi apara warning pentru ca nu il folosesc dar am nevoie la structura de declarare 

    char path[50];
    snprintf(path,sizeof(path),"command_file");
    int f=open(path,O_RDONLY,0777);
    if(f<0)
    {
        perror("ERROR at opening\n");
        exit(-1);
    }

    char command[COM_size]={0};
    if(read(f,command,COM_size-1)<0)//daca esueaza citirea
    {
        perror("Error at reading command_file\n");
        close(f);
        exit(-1);
    }
    command[strcspn(command,"\n")]='\0';//strcspn returneaza pozitia primului '\n',pe care il inlocuieste cu '\0'



    if (strcmp(command, "--list_hunts") == 0) 
    {
        write_in_pipe(pfd_list_hunts);
        list_all_hunts();
        close(STDOUT_FILENO);
        return;
    }
    else if(strstr(command,"--view")!=NULL)
    {
        char *subsir=strtok(command," "); //subsir="--view"
        subsir=strtok(NULL," ");
        char hunt[64];
        strcpy(hunt, subsir);                     
        subsir = strtok(NULL, " ");              
        char id[16];
        strcpy(id, subsir);                       

        //Creăm un proces copil pentru a lansa `treasure_manager`
        write_in_pipe(pfd_view);
        pid_t p = fork();
        if (p < 0) {
           
            perror("ERROR at process\n");
            exit(-1);
        }
        if (p == 0) {
            execlp("./treasure_manager",
                   "treasure_manager",
                   "--view", hunt, id,
                   NULL);
     
            perror("ERROR execlp la view\n");
            exit(1);
        }
         wait(NULL);
         close(STDOUT_FILENO);
         return;
        
    }
   
    else if (strstr(command, "--list") != NULL) {
     
        char *subsir=strtok(command," "); //subsir="--list"
        subsir=strtok(NULL," ");
        char hunt[64];
        strcpy(hunt, subsir);  

        write_in_pipe(pfd_list_treasure);
        pid_t p = fork();
        if (p < 0) {
            perror("Error at process\n");
            exit(-1);
        }
        if (p == 0) {
            execlp("./treasure_manager",
                   "treasure_manager",
                   "--list", hunt,
                   NULL);
            perror("Error at execlp la list\n");
            exit(1);
        }
        wait(NULL);
        close(STDOUT_FILENO);
        return;
    }
    else if(strstr(command,"--calculate_score")!=NULL)
    {
        char *subsir=strtok(command," "); //subsir="--list"
        subsir=strtok(NULL," ");
        char hunt[64];
        strcpy(hunt, subsir);  

        write_in_pipe(pfd_calc);
        pid_t p = fork();
        if (p < 0) {
            perror("Error at process calculate\n");
            exit(-1);
        }
        if (p == 0) {
            execlp("./calculate_score",
                   "calculate_score",
                   hunt,
                   NULL);
            perror("Error at execlp la calculate\n");
            exit(1);
        }
        wait(NULL);//imi blocheaza procesul monitor pana se executa procesul copil lansat de acesta 
        close(STDOUT_FILENO);//inchidem in procesul monitor ca sa ii semnalam hubului(care citeste din pipe) ca nu mai vine nimic adica se trimite EOF
        return;
    }
    else {
        fprintf(stderr, "Monitor: comanda necunoscuta '%s'\n", command);
    }

    close(f);
}

void handle_sigterm(int sig_type)
{
    (void)sig_type;
    exit(-1);//inchidem imediat procesul;
}


//functia care contine bucla de semnale a monitorului(aia care instaleaza sigaction pentru SIGUSR1,SIGTERM
//si dupa se face pause()) 
void monitor_loop()
{
    struct sigaction sa;

    sa.sa_handler=handle_sigusr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags=0;
    if(sigaction(SIGUSR1,&sa,NULL)==-1)
    {
        perror("Error SIGUSR1 action\n");
        exit(-1);
    }

    sa.sa_handler=handle_sigterm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags=0;

    if(sigaction(SIGTERM,&sa,NULL)==-1)
    {
        perror("Error SIGTERM action\n");
        exit(-1);
    }

   // printf("Monitor (PID=%d) apasat, aștept comenzi...\n", getpid());

     while (1) {
        pause();
    }

}

//aceasta functie va porni procesul monitor in background
//se va rula in procesul copil(fork), iar procesului parinte ii va intoarce PID-ul
pid_t start_monitor()
{
    if(monitor_running)
    {
        printf("Monitor already is running (PID= %d)\n",monitor_pid);
        return monitor_pid;
    }
    if(pipe(pfd_list_hunts)<0 || pipe(pfd_list_treasure)<0 || pipe(pfd_view)<0 || pipe(pfd_calc)<0)
    {
        perror("Error at pipe\n");
        exit(-1);
    }
    pid_t pid=fork();//creare proces nou
    if(pid<0) 
    {
        perror("Error at procces\n");
        exit(-1);
    }
    if(pid==0)//ramura copilului (monitorul)
    {
        close(pfd_list_hunts[0]);
        close(pfd_list_treasure[0]);  
        close(pfd_view[0]);
        close(pfd_calc[0]);
        monitor_loop();
        exit(0);
    }
    //ramura parintelui cand pid>0 (codul efectiv al hubului)
    monitor_pid=pid;
    monitor_running=1;

    close(pfd_list_hunts[1]);//inchidem capatul de scriere al pipe-ului
    close(pfd_list_treasure[1]); //inchidem capatul de scriere al pipe-ului
    close(pfd_view[1]); //inchidem capatul de scriere al pipe-ului
    close(pfd_calc[1]); //inchidem capatul de scriere al pipe-ului
    printf("Monitor has started with PID: %d\n",monitor_pid);
    return monitor_pid;
}

void view_treasure()
{
    char hunt[64];
    int id;

    printf("HUNT NAME:");
    if(!fgets(hunt,sizeof(hunt),stdin))//e mai sigur ,impotriva buffer overflow
    {
        perror("Error at reading hunt\n");
        exit(-1);
    }
    hunt[strcspn(hunt,"\n")]='\0';//eliminam '\n' de la sfarsitul huntului 

    printf("Treasure ID:");
    if(scanf("%d",&id)!=1)
    {
        fprintf(stderr,"\nInvalid ID\n");
        while(getchar()!='n');
        return;
    }

    getchar(); //eliminam '\n' ramas de la scanf

    //acum construim comanda 

    char command[COM_size];

    snprintf(command,sizeof(command),"--view %s %d",hunt,id);

    write_command(command);
    kill(monitor_pid,SIGUSR1);//ii zice la kernel ca sa trimita semnalul SIGUSR1 catre procesul cu PID-ul corespunzator
    read_from_pipe(pfd_view[0]);//aici citim din pipe-ul corespunzator
}

void list_hunts()
{
    char command[COM_size];
    snprintf(command,sizeof(command),"--list_hunts");
    write_command(command);
    kill(monitor_pid,SIGUSR1);//trimite semnal la procesul copil(monitor)
    read_from_pipe(pfd_list_hunts[0]);//aici citim din pipe-ul corespunzator
}
void list_treasure()
{
    char hunt[64];
    char command[COM_size];
    printf("Hunt name:");
    if(!fgets(hunt,sizeof(hunt),stdin))
    {
        perror("Error reading hunt\n");
        exit(-1);
    }
    hunt[strcspn(hunt,"\n")]='\0';
    snprintf(command,sizeof(command),"--list %s",hunt);
    
    write_command(command);
    kill(monitor_pid,SIGUSR1);
    read_from_pipe(pfd_list_treasure[0]);//aici citim din pipe-ul corespunzator
}


// pentru faza 3 -- functia de calculate_score
void calculate_score()
{
    char hunt[64];
    char command[COM_size];
    printf("Hunt name:");
    if(!fgets(hunt,sizeof(hunt),stdin))
    {
        perror("Error reading hunt\n");
        exit(-1);
    }
    hunt[strcspn(hunt,"\n")]='\0';
    snprintf(command,sizeof(command),"--calculate_score %s",hunt);
    
    write_command(command);
    kill(monitor_pid,SIGUSR1);
    read_from_pipe(pfd_calc[0]);//aici citim din pipe-ul corespunzator
}



void stop_monitor()
{
    int status;

    printf("Stopping monitor...\n");
    kill(monitor_pid,SIGTERM);
    if(waitpid(monitor_pid,&status,0)<0)
    {
        perror("Error waiting for monitor\n");
        exit(-1);
    }

    if(WIFEXITED(status))
    {
        printf("Monitor exited normally (code %d)\n",WEXITSTATUS(status));

    }
    else if(WIFSIGNALED(status))
    {
        printf("Monitor killed by signal %d\n",WTERMSIG(status));
    }
    else
    {
        printf("Monitor stopped unexpectedly\n");
    }

    monitor_running=0;
    monitor_pid=-1;
}
int main()
{
    
    char choice[16];

    //Pornim mai întâi procesul "monitor" în background
    printf("\n=== Starting monitor ===\n");
    start_monitor();
        //asta e procesul parinte si monitorul ruleaza in copil(o ramas in pause si asteapta semnale)
    //Bucla principală de interacțiune cu utilizatorul
    while (1) {
        //Afișăm meniul
        printf("\n=== HUB MENU ===\n");
        printf("1) List all hunts\n");
        printf("2) List treasures in a hunt\n");
        printf("3) View a treasure by ID\n");
        printf("4) Calculate score\n");
        printf("5) Stop monitor and exit\n");
        printf("\nSelect option: ");

        //Citim alegerea (o linie simplă)
        if (!fgets(choice, sizeof(choice), stdin)) {
            // EOF sau eroare → ieșim din buclă
            break;
        }
        choice[strcspn(choice, "\n")] = '\0';

        //Apelam functiile in funcție de alegerea utilizatorului
        if (strcmp(choice, "1") == 0) {
            list_hunts();
        }
        else if (strcmp(choice, "2") == 0) {
            list_treasure();
        }
        else if (strcmp(choice, "3") == 0) {
            view_treasure();
        }
        else if (strcmp(choice, "4") == 0) {
            calculate_score();
        }
        else if (strcmp(choice, "5") == 0) {
            stop_monitor();
            break;
        }
        else {
            printf("Invalid option: %s\n", choice);
        }
    }

    //Dacă am ieșit din buclă fără să comandăm stop,
    //ne asigurăm că monitorul nu rămâne zombie
    if (monitor_running) {
        stop_monitor();
    }

    return 0;
}