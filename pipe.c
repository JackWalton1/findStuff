#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>
#include <termios.h>

int fd[10][2];
void siginthandler(int signo);
void signalAndFDChanges(int iteration);

int main(){
    signal(SIGUSR1, siginthandler);
    int pid;
    int p_pid = getpid();
    int* bytes_input = (int*)mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    int iteration = 0;
    do{
        signalAndFDChanges(iteration);
        pipe(fd[iteration]);
        //close(fd[0]); // close output end
        char input[100];
        *bytes_input = read(STDIN_FILENO, input, 100);
        printf("Bytes read by stdin: %d\n", *bytes_input);
        fflush(STDIN_FILENO);

        int bytes = write(fd[iteration][1], input, *bytes_input);
        printf("Bytes written: %d\n", bytes);
        pid = fork();
        if(!pid)iteration ++;
    }while(pid);
    //close(fd[1]); // close input end
    if(!pid){
        char text[10];
        char message[200];
        char sub_message[5];
        //pipe(fd);
        int bytes = read(fd[iteration][0], text, *bytes_input);
        strcpy(message, "Bytes read: ");
        sprintf(sub_message, "%d", bytes);
        strcpy(message, sub_message);
        strcat(message, "\n");
        int bytes2 = write(fd[iteration][1], message, strlen(message)+1);
        //printf("Bytes read: %d\n", bytes);

        //close(fd[iteration][1]); // close input end
        //close(fd[iteration][0]); // close output end
        fprintf(stderr, "%s\n", text);

        kill(p_pid, SIGUSR1);

        return 0;
    }
    return 0;
}

void siginthandler(int signo){
    if(signo == SIGUSR1){
        printf("Signal caught!\n");
    }
    return;
}

void signalAndFDChanges(int iteration){
    signal(SIGUSR1, siginthandler);
    //pipe(fd[iteration]);
    int fd_in_copy = dup2(fd_in_copy, fd[iteration][1]);

    printf("%d\n", fd[iteration][1]);
    printf("%d\n", fd_in_copy);

    fd[iteration][0] = STDIN_FILENO;
    char text[1000];
    read(fd[iteration][0], text, 100);
    printf("%s", text);
}