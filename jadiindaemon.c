#include<stdio.h> 
#include<stdlib.h> 
#include<stdbool.h> 
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
time_t old_time;
typedef struct a {int i;} arg; 
typedef struct com              // menampung command yg harus dijalankan sama kapan harus dijalankan
{ 
    char command[20];      // command misal /bin/sh
    char path_prog[100];   // tempat file /home/user/.../blabla.sh
    struct tm time_exec;      // waktu kapan harus dieksekusi
} cmd;

char isi_file[100];              // kumpulan command yg dijalankantaro disini

void *Exec(void *args)  // di check_time panggil fungsi ini
{
    arg *tmp = (arg*) args;
    int i = tmp->i;

    int fd[2], status;
    pid_t pid;
    char result[100];

    pipe(fd);
    if ( (pid = fork() ) == -1)
    {
        fprintf(stderr, "FORK failed");
        return NULL;
    } 
    else if( pid == 0) 
    {
        dup2(fd[1], 1);
        close(fd[0]);
        execlp("/bin/sh","/bin/sh",cmdtest[i],NULL);
    }
    wait(NULL);
    read(fd[0], result, sizeof(result));
    printf("%s",result);

    return NULL;
}

void read_conf()
{
    FILE *fp = fopen(check, "r");
    cmd isi;

        int count = 0;
        char word[100];

        while (fscanf(fp, "%s", word) != EOF) {
            if(++count==7){
                 strcpy(isi.path_prog,word);
                 printf("%s\n", isi.path_prog);
                 count = 0;
            }
            if(count==6){
                 strcpy(isi.command,word);
                 printf("%s\n", isi.command);
            }
        }
    fclose(fp);

    FILE *sp = fopen(check, "r");
        fgets(isi_file, 60, sp);
        isi_file[strlen(isi_file)] = '\0';
        printf("%s\n", isi_file );
    fclose(sp);
}

void check_time()
{
    //check waktu setiap struct com di array
    // sementara bikin aja yg bisa ngecek * * * * *, yg benernya gw yg bikin
    // jika waktunya utk dijalankan panggil thread dgn fungsi Exec() yg diatas utk menjalankan program
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

  while(1) {
    // main program here
    struct stat st;
        stat(check,&st);
        time_t newTime = sb.st_mtime;;
        if (difftime(newTime,old_time) > 0)
        {
            old_time =newTime;
            //read_conf() --> fungsi buat baca config
            read_conf();
        }
        //check_time()  --> fungsi buat check apakah command udh waktunya diexec
    sleep(1);
  }
  
  exit(EXIT_SUCCESS);
}
