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
pthread_t tid[3];
const char cmdtest[2][100] = {"/home/paksi/abc.sh","/home/paksi/cde.sh"};
const char check[100] = {"/home/paksi/config.crontab"};
#define MI (1 << 0)
#define H (1 << 1)
#define D (1 << 2)
#define MO (1 << 3)
#define W (1 << 4)
#define C (1 << 5)
#define P (1 << 6)

int n = 0;
time_t t;
struct tm currT;
typedef struct com
{
    char flags;
    struct tm execT;
    char cm[100];
    char ex[100];
}cmd;

cmd doThis[10];

char isi_file[100];              // kumpulan command yg dijalankantaro disini

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

void setCmd(cmd *curr, char *conv, char flag)
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
        }
        else if (conv[0] == '*')
        {
            curr->flags |= MI;
            curr->execT.tm_min += 1;     
        }
        break;
    case H:

        if (isdigit(conv[0]))
        {
            char *end;
            int num = strtol(conv,&end,10);
            if (num > -1 && num < 24) curr->execT.tm_hour = num;
            if ((curr->flags & MI) && curr->execT.tm_hour != currT.tm_hour) curr->execT.tm_min = 0;
        }
        else if (conv[0] == '*')
        {
            curr->flags |= H;
            if (!(curr->flags & MI)) curr->execT.tm_hour += 1;
        }
            
        break;
    case D:
        if (isdigit(conv[0]))
        {
            char *end;
            int num = strtol(conv,&end,10);
            if (num > -1 && num < 32) curr->execT.tm_mday = num;
            if  (curr->execT.tm_mday != currT.tm_mday) 
            {
                if (curr->flags & H) curr->execT.tm_hour = 0;
                if (curr->flags & MI) curr->execT.tm_min = 0;
            }
        }
        else if (conv[0] == '*')
        {
            curr->flags |= D;
            if ( !(curr->flags & H) && curr->execT.tm_hour <= currT.tm_hour) 
                if ( curr->execT.tm_min < currT.tm_min) curr->execT.tm_mday += 1; 
            else if (!( curr->flags & (H | MI) ) ) curr->execT.tm_mday += 1;
        }
        break;
    case MO:
        if (isdigit(conv[0]))
        {
            char *end;
            int num = strtol(conv,&end,10);
            if (num > 0 && num < 13) curr->execT.tm_mon = num -1;
            if (curr->execT.tm_mon != currT.tm_mon) 
            {
                if (curr->flags & D) curr->execT.tm_mday = 1;
                if (curr->flags & H) curr->execT.tm_hour = 0;
                if (curr->flags & MI) curr->execT.tm_min = 0; 
            }
            if (curr->execT.tm_mon < currT.tm_mon) curr->execT.tm_year += 1; 
            else if ( !(curr->flags & D) && curr->execT.tm_mday <= currT.tm_mday) 
                     if (  curr->execT.tm_hour < currT.tm_hour)  curr->execT.tm_year += 1;
            else if (!(curr->flags & (D | H | MI)))  curr->execT.tm_year += 1;  
        }
        else if (conv[0] == '*')
        {
            curr->flags |= MO;
            if ( !(curr->flags & D) && curr->execT.tm_mday <= currT.tm_mday) 
                if ( curr->execT.tm_hour < currT.tm_hour) curr->execT.tm_mon += 1;
            else if (!(curr->flags & (D | H | MI)))  curr->execT.tm_mon += 1;
        }
            
        break;
    case W:
        if (isdigit(conv[0]))
        {
            curr->flags |= W;
            char *end;
            int num = strtol(conv,&end,10);
            if (num > -1 && num < 7) curr->execT.tm_wday = num;
        }
        break;
    case C:
        if ((strcmp(conv,"bash")) == 0)
        {
            strcpy(curr->cm,"/bin/sh");
            curr->flags |= C;
        }
        else if ((strstr(conv,"/")) != NULL)
        {
            strcpy(curr->cm,conv);
            curr->flags |= C;
        }
        
        break;
    case P:
        if (curr->flags & P)
        {
            sprintf(curr->ex,"%s %s",curr->ex, conv);
        } 
        else if ((strstr(conv,"/")) != NULL)
        {
            sprintf(curr->ex,"%s %s",curr->cm, conv);
            curr->flags |= P;
        } 
        break;
    default:
        break;
  }
}

void check_time()
{   
    time_t raw;
    struct tm * info;
    int i;
    time ( &raw );
    info = localtime ( &raw );
    for(i=0 ; i<=n; i++){
    if(info->tm_min==doThis[i].execT.tm_min&&info->tm_hour==doThis[i].execT.tm_hour&&info->tm_mday==doThis[i].execT.tm_mday&&info->tm_mon==doThis[i].execT.tm_mon){
        printf("%d %d %d\n",doThis[i].execT.tm_min, info->tm_sec,info->tm_min);
         incTime(&doThis[i]);  
         printf("%d %d %d\n",doThis[i].execT.tm_min, info->tm_sec,info->tm_min);
         system(doThis[i].ex);
    }
    }
    //check waktu setiap struct com di array
    // sementara bikin aja yg bisa ngecek * * * * *, yg benernya gw yg bikin
    // jika waktunya utk dijalankan panggil thread dgn fungsi Exec() yg diatas utk menjalankan program
}

void read_conf()
{
    FILE *fp;
    fp = fopen(check,"r");
    char read[10][100];
    t = time(NULL);
    currT = *localtime(&t);
    int stage=0;
    char word[100],enter[2];
    

    while (fscanf(fp, "%s%c", word, enter) == 2) {
        
        if (stage == 0)
        {
            doThis[n].flags = 0;
            doThis[n].execT = currT;
        }
 
        setCmd(&doThis[n],word, (1 << stage));
        if (stage < 6) stage++;
        if (enter[0] == '\n')
        {
            stage = 0;
            puts(asctime(&doThis[n].execT));
            puts(doThis[n].cm);
            puts(doThis[n].ex);
            n++;
        }
        memset(word,'\0',sizeof(word));
        
    }
    fclose(fp);
}


int main() {
  pid_t pid, sid;
    struct stat sb;
    stat(check,&sb);
    time_t old_time = sb.st_mtime;

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
        time_t newTime = sb.st_mtime;;
        if (difftime(newTime,old_time) > 0)
        {
            old_time =newTime;
            read_conf();
        }
        check_time();
        //check_time()  --> fungsi buat check apakah command udh waktunya diexec
    sleep(1);
  }
  
  exit(EXIT_SUCCESS);
}
