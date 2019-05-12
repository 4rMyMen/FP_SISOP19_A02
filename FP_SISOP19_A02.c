#include<stdio.h> 
#include<ctype.h>
#include<stdbool.h> 
#include<stdlib.h> 
#include<string.h> 
#include<unistd.h> 
#include <pthread.h>
#include <dirent.h>
#include<fcntl.h> 
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/types.h>
const char check[100] = {"/home/bimo/Desktop/testingfp/config.crontab"};
#define MI (1 << 0)
#define H (1 << 1)
#define D (1 << 2)
#define MO (1 << 3)
#define W (1 << 4)
#define C (1 << 5)
#define P (1 << 6)
#define PI (1 << 7)




typedef struct com
{
    __u_char flags;
    struct tm execT;
    char cm[100];
    char ex[100];
    char pip[200];
}cmd;
typedef struct config
{
    int n;
    time_t t;
    struct tm currT;
    cmd *doThis;

}conf;

conf con;

void incTime(cmd *curr)
{
    curr->execT.tm_sec =0; 

    if (curr->flags & MI)
    {
        curr->execT.tm_min += 1;
        if(curr->execT.tm_min == 60)
        {
            curr->execT.tm_min = 0;
            if (curr->flags & H) curr->execT.tm_hour += 1;
            else if (curr->flags & D) curr->execT.tm_mday += 1;
            else if (curr->flags & MO) curr->execT.tm_mon += 1;
            else curr->execT.tm_year += 1; 
        }
    }        
    if (curr->flags & H)
    {
        if (!(curr->flags & MI)) curr->execT.tm_hour += 1;
        if(curr->execT.tm_hour == 24)
        {
            curr->execT.tm_hour = 0;
            if (curr->flags & D) curr->execT.tm_mday += 1;
            else if (curr->flags & MO) curr->execT.tm_mon += 1;
            else curr->execT.tm_year += 1; 
        }
    }        
    if (curr->flags & D)
    {
        if (!( curr->flags & (H | MI) ) ) curr->execT.tm_mday += 1;
        mktime(&curr->execT);
        if (!(curr->flags & MO) && curr->execT.tm_mday == 1)
        {
            curr->execT.tm_year += 1; 
            curr->execT.tm_mon -= 1;
            mktime(&curr->execT);
        }
    }
    if (curr->flags & MO)
    {
        if (!(curr->flags & (D | H | MI)))  curr->execT.tm_mon += 1;
        mktime(&curr->execT);
    }
    else
    {
        if (!(curr->flags & (D | H | MI)))  curr->execT.tm_year += 1; 
    }

}

int setCmd(cmd *curr, char *conv, __u_char flag)
{
    curr->execT.tm_sec =0; 
    switch (flag)
    {
    case MI:
        if (isdigit(conv[0]))
        {
            char *end;
            int num = strtol(conv,&end,10);
            if (num > -1 && num < 60) curr->execT.tm_min = num;
            else return 0;

        }
        else if (conv[0] == '*')
        {
            curr->flags |= MI;
            curr->execT.tm_min += 1;     
        }
        else return 0;
        break;
    case H:

        if (isdigit(conv[0]))
        {
            char *end;
            int num = strtol(conv,&end,10);
            if (num > -1 && num < 24) curr->execT.tm_hour = num;
            else return 0;
            if ((curr->flags & MI) && curr->execT.tm_hour != con.currT.tm_hour) curr->execT.tm_min = 0;
        }
        else if (conv[0] == '*')
        {
            curr->flags |= H;
            if (!(curr->flags & MI) && curr->execT.tm_min < con.currT.tm_min) curr->execT.tm_hour += 1;
        }
        else return 0;
        break;
    case D:
        if (isdigit(conv[0]))
        {
            char *end;
            int num = strtol(conv,&end,10);
            if (num > -1 && num < 32) curr->execT.tm_mday = num;
            else return 0;
            if  (curr->execT.tm_mday != con.currT.tm_mday) 
            {
                if (curr->flags & H) curr->execT.tm_hour = 0;
                if (curr->flags & MI) curr->execT.tm_min = 0;
            }
        }
        else if (conv[0] == '*')
        {
            curr->flags |= D;
            if ( !(curr->flags & H) && curr->execT.tm_hour <= con.currT.tm_hour) 
                if ( curr->execT.tm_min < con.currT.tm_min) curr->execT.tm_mday += 1; 
            else if (!( curr->flags & (H | MI) ) 
                && curr->execT.tm_hour < con.currT.tm_hour) curr->execT.tm_mday += 1;
        }
        else return 0;
        break;
    case MO:
        if (isdigit(conv[0]))
        {
            char *end;
            int num = strtol(conv,&end,10);
            if (num > 0 && num < 13) curr->execT.tm_mon = num -1;
            else return 0;
            if (curr->execT.tm_mon != con.currT.tm_mon) 
            {
                if (curr->flags & D) curr->execT.tm_mday = 1;
                if (curr->flags & H) curr->execT.tm_hour = 0;
                if (curr->flags & MI) curr->execT.tm_min = 0; 
            }
            if (curr->execT.tm_mon < con.currT.tm_mon) curr->execT.tm_year += 1; 
            else if ( !(curr->flags & D) && curr->execT.tm_mday <= con.currT.tm_mday) 
                     if (  curr->execT.tm_hour < con.currT.tm_hour)  curr->execT.tm_year += 1;
            else if (!(curr->flags & (D | H | MI)) 
                && curr->execT.tm_mon < con.currT.tm_mon)  curr->execT.tm_year += 1;  
        }
        else if (conv[0] == '*')
        {
            curr->flags |= MO;
            if ( !(curr->flags & D) && curr->execT.tm_mday <= con.currT.tm_mday) 
                if ( curr->execT.tm_hour < con.currT.tm_hour) curr->execT.tm_mon += 1;
            else if (!(curr->flags & (D | H | MI)) 
                && curr->execT.tm_mday < con.currT.tm_mday)  curr->execT.tm_mon += 1;
        }
        else return 0;          
        break;
    case W:
        if (isdigit(conv[0]))
        {
            char *end;
            int num = strtol(conv,&end,10);
            if (num > -1 && num < 7) curr->execT.tm_wday = num;
            else return 0;
        }
        else if (conv[0] == '*')
            curr->flags |= W;
        else return 0;
        break;
    case C:
        if ((strstr(conv,"/")) != NULL)
        {
            struct stat tmp;
            if(stat(conv,&tmp) != 0 ) return 0;
            strcpy(curr->ex,conv);
            curr->flags |= P;
        }
        else return 0;
        break;
    case P:
        if ((strstr(conv,"/")) != NULL)
        {
            struct stat tmp;
            if(stat(conv,&tmp) != 0 )
            { 
                curr->flags &= ~P;
                return 0;
            }
            strcpy(curr->cm,curr->ex);
            memmove(curr->ex,conv,strlen(conv) + 1);
            curr->flags |= C;
        }
        else if (conv[0] == '|' || conv[0] == '>')
        {
            strcpy(curr->pip,conv);
            curr->flags |= PI;
        }
        else return 0;       
        break;
    case PI:
        if (curr->flags & PI)
            sprintf(curr->pip,"%s %s",curr->pip,conv);
        else if (conv[0] == '|' || conv[0] == '>')
        {
            strcpy(curr->pip,conv);
            curr->flags |= PI;
        }
        else return 0;
        break;         

    default:
        break;
  }
  mktime(&curr->execT);
  return 1;
}

void Exec(cmd *curr) 
{

    int fd[2];
    pid_t pid;


    pipe(fd);
    if ( (pid = fork() ) == -1)
    {
        fprintf(stderr, "FORK failed");
        return;
    } 
    else if( pid == 0) 
    {
        dup2(fd[1], 1);
        close(fd[0]);
        if (curr->flags & C)
            execlp(curr->cm,curr->cm,curr->ex,NULL);
        else execlp(curr->ex,curr->ex,NULL);
    }
    wait(NULL);

    return;
}

void *check_com(void* arg)
{
    cmd *curr =  (cmd*) arg;
    time_t raw;
    struct tm * info;
    char w = 'n';
    printf("there\n");
    time ( &raw );
    info = localtime ( &raw );

    if(info->tm_min != curr->execT.tm_min) return NULL;      
    if(info->tm_hour != curr->execT.tm_hour) return NULL;     
    if(info->tm_mday != curr->execT.tm_mday) 
    {
        if (!(curr->flags & W))
        {
            if (info->tm_wday != curr->execT.tm_wday) return NULL;
            else w = 'y';
        }  
        else
        {
            if (curr->execT.tm_mday == 29 && curr->execT.tm_mon == 1 && info->tm_year > curr->execT.tm_year)
                curr->execT.tm_year +=1;
            return NULL;
        } 
    }      
    if(info->tm_mon != curr->execT.tm_mon) return NULL;
    Exec(curr);
    if (w != 'y') incTime(curr);
    printf("done\n");
    return NULL;
}

void check_time()
{   
    int i;
           
    pthread_t *tid = (pthread_t*)malloc(con.n * sizeof(pthread_t));
     printf("here\n");
    for(i=0 ; i<con.n; i++)
    {
        pthread_create(&tid[i],NULL,  &check_com, (void*) &con.doThis[i]);
    }
    for (int i = 0; i < con.n; i++)
    {
        pthread_join(tid[i],NULL);
    }
    free(tid);
}


void read_conf()
{
    con.n =0;
    FILE *fp,*ap;
    ap = fopen(check,"a+");
    fputs("\n",ap);
    fclose(ap);
    fp = fopen(check,"r");
    con.t = time(NULL);
    con.currT = *localtime(&con.t);
    int stage=0, i=1, reloc = 0;
    char word[100],enter[2];
    memset(word,'\0',sizeof(word));
    int status = 1;
    cmd *arrCmd = (cmd*)malloc(10*sizeof(cmd));
    cmd curr;
    while (fscanf(fp, "%s%c", word, enter) == 2) 
    {
        
        if (stage == 0)
        {
            cmd empty;
            curr = empty;
            curr.flags = 0;
            curr.execT = con.currT;
        }
        if (status == 1)
        {
            status = setCmd(&curr,word, (1 << stage));
            if (stage < 7) stage++;
        }
        if (enter[0] == '\n')
        {
            stage = 0;
            status = 1;
            if (!(curr.flags & P)) continue;
            puts(asctime(&curr.execT));
            puts(curr.cm);
            puts(curr.ex);
            puts(curr.pip);
            if (con.n % 9 == 0)
            {
                i++;
                cmd *temp = arrCmd;
                arrCmd = (cmd*)realloc(arrCmd,i*10*sizeof(cmd));
                if(arrCmd == NULL)
                {
                    arrCmd = temp;
                    free(temp);
                }
            }
            arrCmd[con.n] = curr;
            con.n++;
        }
        con.doThis = arrCmd;

        memset(word,'\0',sizeof(word));
        
    }
    free(arrCmd);
    fclose(fp);
}


int main() {
    pid_t pid, sid;
    struct stat sb;
    stat(check,&sb);
    con.t = sb.st_mtime;

  pid = fork();

  if (pid < 0) {
    exit(EXIT_FAILURE);
  }

  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  umask(0);

  sid = setsid();

  if (sid < 0) {
    exit(EXIT_FAILURE);
  }

  if ((chdir("/")) < 0) {
    exit(EXIT_FAILURE);
  }

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

    read_conf();
  while(1) {
    // main program here
    struct stat st;
    stat(check,&st);
    time_t newTime = st.st_mtime;
    if (difftime(newTime,con.t) > 0)
    {
        con.t =newTime;
        read_conf();
    }
    check_time();
    sleep(1);
  }
  
  exit(EXIT_SUCCESS);
}
