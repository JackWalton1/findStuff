#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <signal.h>
#include <termios.h>
#include <sys/resource.h>
#include <sys/time.h>

// TEST 10 CHILDREN OVERLOAD TO SEE IF ACTIVE CHILD # IS ACCURATE (PRETTY ACCURATE)
// MAKE PIPES WORK WITH PARENT.... PRINTFS BECOME WRITES AND SIGNAL CHANGES
// FD[1] TO STDIN AND FD[0] TO STDOUT?

void printStats(const char * input);
void listFilesRecursively(char *basePath);
void searchFilesRecursively(char *basePath, char* paths_sucessful, char* lookup);
void directSearchFilesRecursively(char *basePath, char* paths_sucessful, char* lookup);
void directSpecificSearchFilesRecursively(char *basePath, char* paths_sucessful, char* lookup, char* specific);
int EndingEqualsTail(char *filename, char* tail);
void siginthandler(int signo);
void createChildMessage(int next_cid, char* child_message_in, char *input);

char input[100]; // global, so easier to clear from parent or child
int fd[10][2]; // pipes for each child (for list)

int main(){
    int pid;
    int p_pid = getpid();
    signal(SIGINT, siginthandler);
    char* child1message = (char*)mmap(NULL, sizeof(char)*100, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    char* child2message = (char*)mmap(NULL, sizeof(char)*100, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    char* child3message = (char*)mmap(NULL, sizeof(char)*100, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    char* child4message = (char*)mmap(NULL, sizeof(char)*100, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    char* child5message = (char*)mmap(NULL, sizeof(char)*100, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    char* child6message = (char*)mmap(NULL, sizeof(char)*100, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    char* child7message = (char*)mmap(NULL, sizeof(char)*100, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    char* child8message = (char*)mmap(NULL, sizeof(char)*100, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    char* child9message = (char*)mmap(NULL, sizeof(char)*100, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    char* child10message = (char*)mmap(NULL, sizeof(char)*100, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    char** childMessages = (char**)mmap(NULL, sizeof(char*)*10, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    childMessages[0] = child1message;
    childMessages[1] = child2message;
    childMessages[2] = child3message;
    childMessages[3] = child4message;
    childMessages[4] = child5message;
    childMessages[5] = child6message;
    childMessages[6] = child7message;
    childMessages[7] = child8message;
    childMessages[8] = child9message;
    childMessages[9] = child10message;
    //{child1message, child2message, child3message, child4message, child5message, child6message, child7message, child8message, child9message, child10message};


    //int list_pids[10]; //i.e. index 1 = child one [pidC1, pidC2...pidC10]
    int* list_pids = (int*)mmap(NULL, sizeof(int)*10, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    memset(&list_pids[0], 0, sizeof(*list_pids)); // sets to list of 0's
    int* active_children = (int*)mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    *active_children = 0; // keep track of how many searches are going on at once
    int* cid = (int*)mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    *cid = -1; // index of next child 
    char path[100]; //cwd
    char *token; // to split up input
    char words[5][20]; // array of input words
    int is_direct = 0; // this and next few lines are booleans for arguments
    int is_recursive = 0;
    int is_specific = 0;
    char paths_successful[3000];//keep list of all paths where search is found
    int fd[10][2]; // pipes for each child (for list)
    // array for pipes, so on "list" we can print what they're doing
    do{
        memset(&paths_successful[0], 0, sizeof(paths_successful)); // clear successful paths
        getcwd(path, 100);
        fprintf(stderr, "\nEnter a command of max 100 chars\n"); //print the pretty prompt 
        fprintf(stderr, "\033[0;34m");
        fprintf(stderr, "findStuff ");
        fprintf(stderr, "%s", path);
        fprintf(stderr, "\033[0m");
        fprintf(stderr, "$ ");
        read(STDIN_FILENO, input, 100);
        // SPLITTING UP INPUT
        // loop through the string to extract all other tokens
        token = strtok(input, " \t\r\n\v\f");
        int i = 0;
        while(token != NULL) {
            strcpy(words[i], token); //store strings in array
            token = strtok(NULL, " \t\r\n\v\f");
            i++;
        }
        for(i; i<5;i++){
            memset(&words[i], 0, sizeof(words[i]));
        }
        if(*cid >= 9){
            *cid = 0;
        }
        else{
            *cid = *cid +1;
        }
        *active_children = *active_children + 1;
        if(*active_children>9){
            printf("\nSorry there are too many children. Try again when one finishes.\n");
        }
        else{
            pid = fork();
        }
    }while(pid);

    // FORK INTO CHILD PROCESS
if(!pid){
    list_pids[*cid] = getpid();
    //if((strcmp(words[0], "list")!=0)||words[0][0]!='q'){sleep(10);}
    //if(strcmp(words[0], "find")==0){ sleep(10);}
    //sleep(20);


    //FIND (WORKS)
    if(strcmp(words[0], "find")==0){
        // pipe(fd[*cid]); //I know that this is the right pipe...
        // //PROGRAM IS EXITING OUT HERE^ WHEN I ADD BUGGY LINE
        // close(fd[*cid][0]); // Close the ouput, because I will only write
        //start clock
        struct timeval start, end;
        gettimeofday(&start, NULL);

        char child_message_in[100]; //store message here
        createChildMessage(*cid, child_message_in, words[1]);
        strcpy(childMessages[*cid], child_message_in);
        sleep(3);
        //printf("%s",child_message_in );
    /*LINE BELOW MAKES MY DEBUGGER GO INSANE...CANNOT FIGURE OUT WHY I CANNOT WRITE. 
    IF I DO LIST BEFORE FIND THE PIPES OPEN FINE AND reads() NOTHING (OBVIOUSLY) */
        // int bytes_sent = write(fd[*cid][1], child_message_in, strlen(child_message_in)+1);
        // //printf("%d\n", bytes_sent); 
        // close(fd[*cid][1]);
        //sleep(20); FOR TESTING KILL/LIST/Q

        //ARG HANDLING
        if(words[1][0] == '"'){
            // remove quotes for direct input
            words[1][strlen(words[1])-1] = '\0';
            memcpy(words[1], words[1]+1, sizeof(words[1]));
            is_direct = 1;
        }
        if(strcmp(words[2], "-s")==0 ||strcmp(words[3], "-s")==0){
            is_recursive = 1;
        }
        int is_specific_loc = -1;
        if(strncmp(words[2], "-f", 2)==0){
            is_specific = 1;
            is_specific_loc = 2;
        }
        if(strncmp(words[3], "-f", 2)==0){
            is_specific = 1;
            is_specific_loc = 3;
        }

        // ACTUAL ALGOS
        DIR *d;
        struct dirent *dir;
        d = opendir(".");
        // IF SEARCHING FOR DIRECT WORD
        if(is_direct){
            //look at file contents, search for words[1]
            int search_width = strlen(words[1]);
            if(d){
                if(is_specific && is_recursive){
                    memcpy(words[is_specific_loc], words[is_specific_loc]+3, sizeof(words[is_specific_loc]));
                    directSpecificSearchFilesRecursively(".", paths_successful, words[1], words[is_specific_loc]);
                    // printf("Locations found: \n%s", paths_successful);
                }
                else if(is_recursive){
                    directSearchFilesRecursively(".", paths_successful, words[1]);
                    // printf("Locations found: \n%s", paths_successful);
                }
                else{
                     if (is_specific){
                            memcpy(words[is_specific_loc], words[is_specific_loc]+3, sizeof(words[is_specific_loc]));
                     }
                    while((dir = readdir(d)) != NULL) {
                        int found = 0;
                        //if(is_specific)
                        if (is_specific){
                            if(strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0 && (EndingEqualsTail(dir->d_name, words[is_specific_loc]) || dir->d_type == DT_DIR)){
                                //Actually continue to printf
                            }
                            else{
                                //Continue to next while loop iteration
                                continue;
                            }
                        }
                        
                        //printf("\nSearching: %s for %s\n", dir->d_name, words[1]);
                        FILE* inPtr = fopen(dir->d_name, "r");
                        int correct_chars = 0;
                        char ch;
                        while((ch = fgetc(inPtr)) != EOF && !found){
                            if(ch == words[1][correct_chars]){
                                correct_chars++;
                            }
                            else{
                                correct_chars = 0;
                            }
                            if(correct_chars == search_width){
                                found = 1;
                            }
                        }
                        if(found){
                            //printf("\nFound the phrase: %s\n", words[1]);
                            strcat(paths_successful, "./");
                            strcat(paths_successful, dir->d_name);
                            strcat(paths_successful, "\n");
                        }
                        fseek(inPtr, 0, SEEK_SET);
                        fclose(inPtr);
                    }
                    // printf("Locations found: \n%s", paths_successful);
                }
                if(strlen(paths_successful)==0){
                    printf("\nThe direct phrase: %s was not found.\n", words[1]);
                }
                else{
                    printf("\nSearch for the direct phrase \"%s\" finished. \nLocations found: \n%s\n", words[1], paths_successful);
                }
            }
        }
        //ELSE SEARCH FOR FILE NAME
        else{
            if(d){
                if(is_recursive){
                    searchFilesRecursively(".", paths_successful, words[1]);
                    //printf("\nFound the file called: %s\n", words[1]);
                    if(strlen(paths_successful) == 0){
                        printf("\n%s not found in CWD or it's subdirectories\n", words[1]);
                    }
                }
                else{
                    while((dir = readdir(d)) != NULL) {
                        if(strcmp(dir->d_name, words[1])==0){
                            //printf("\nFound the file called: %s\n", dir->d_name);
                            strcat(paths_successful, "./");
                            strcat(paths_successful, dir->d_name);
                            strcat(paths_successful, "\n");
                        }
                    }
                    if(strlen(paths_successful) == 0){
                        printf("\n%s not found in CWD\n", words[1]);
                    }
                }
                if(strlen(paths_successful) > 0){
                    printf("\nSearch for %s Finished. Locations found: \n%s\n", words[1], paths_successful);
                }
            }
        }
        closedir(d);
        gettimeofday(&end, NULL);
 
        long seconds = (end.tv_sec - start.tv_sec);
        long micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);
        if(seconds<10){
            printf("Time taken for search(HH:MM:SS.MS):\n 00:00:0%d.%d\n", *cid, seconds, micros);
        }
        else{
            printf("Time taken for search(HH:MM:SS.MS):\n 00:00:%d.%d\n", seconds, micros);

        }

    }
    //LIST (CHILD WON'T WRITE in "FIND" ifBLK SO IT NEVER READS ANYTHING)
    else if(strcmp(words[0], "list")==0){
        //read all the child messages from pipes
        for(int k = 0; k<10; k++){
            //char child_message_out[100];

            printf("%s", childMessages[k]);
            memset(childMessages[k], 0, sizeof(childMessages[k]));
            //pipe(fd[k]);//open pipe 
            //close(fd[k][1]);//close input
            //int bytes_read = read(fd[k][0], child_message_out, sizeof(char)*100); //read output
            // if(bytes_read > 1){
            //     printf("%s\n", child_message_out);
            // }
            // close(fd[k][0]);//close output
        }

    }
    //KILL (KINDA WORKING)
    else if(strcmp(words[0], "kill")==0){
        int c_index = atoi(words[1]);
        //printf("\nACTUAL: %d | LIST_V: %d\n", getpid(), list_pids[c_index-1]); // SHOULDNT BE THE SAME
        memset(childMessages[c_index-1], 0, sizeof(childMessages[c_index-1]));
        kill(list_pids[c_index-1], SIGKILL);
    }
    //QUIT (WORKS)
    else if(input[0] == 'q'){
        //kill all processes, incl parent
        int j = 0;
        int surviving_pid = 0;
        munmap(cid, sizeof(int));
        munmap(active_children, sizeof(int));
        //UNMAP ALL THE CHILD MESSAGES
        munmap(childMessages, sizeof(char*)*10);
        munmap(child1message, sizeof(char)*100);
        munmap(child2message, sizeof(char)*100);
        munmap(child3message, sizeof(char)*100);
        munmap(child4message, sizeof(char)*100);
        munmap(child5message, sizeof(char)*100);
        munmap(child6message, sizeof(char)*100);
        munmap(child7message, sizeof(char)*100);
        munmap(child8message, sizeof(char)*100);
        munmap(child9message, sizeof(char)*100);
        munmap(child10message, sizeof(char)*100);

        for(j;j<10;j++){
            if(list_pids[j] == getpid()){
                surviving_pid = list_pids[j];
            }
            else if(list_pids[j]!= 0){
                kill(list_pids[j], SIGKILL);
            }
        }
        munmap(list_pids, sizeof(int)*10);
        //kill parent and then self
        kill(p_pid, SIGKILL);
        kill(surviving_pid, SIGKILL);
        return 0;
    }
    // ALL CASES BESIDES QUIT SEND SIGNAL & REDUCE ACTIVE # WHEN DONE
    //printf("child pid: %d\n", list_pids[*cid]);
    //strcpy(childMessages[*cid], "\0");
    memset(childMessages[*cid], 0, sizeof(childMessages[*cid]));
    //printf("\nNOT ACTUAL: %s", *childMessages[*cid]);
    *active_children = *active_children -1;
    kill(p_pid, SIGINT);
    //kill(list_pids[*cid], SIGKILL);
    return 0;    
}

    //on the off chance parent gets here it returns from main
    else{
        return 0;
    }
    

}

// A FEW SEARCH FUNCTIONS FOR THE DIFFERENT FLAGS
void listFilesRecursively(char *basePath){
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    // Unable to open directory stream
    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            printf("%s\n", dp->d_name);

            // Construct new path from our base path
            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);

            listFilesRecursively(path);
        }
    }

    closedir(dir);
}

void searchFilesRecursively(char *basePath, char* paths_sucessful, char* lookup){
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    // Unable to open directory stream
    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL){

        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0){
            //printf("%s\n", dp->d_name);
            // Construct new path from our base path
            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);
            if(strcmp(lookup, dp->d_name)==0){
                strcat(paths_sucessful, path);
                strcat(paths_sucessful, "\n");
            }

            searchFilesRecursively(path, paths_sucessful, lookup);
        }
    }

    closedir(dir);
}

void directSearchFilesRecursively(char *basePath, char* paths_sucessful, char* lookup){
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    // Unable to open directory stream
    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL){

        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0){

            int found = 0;
            //printf("\nSearching: %s for %s\n", dp->d_name, lookup);
            int search_width = strlen(lookup);

            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);

            FILE* inPtr = fopen(path, "r");
            if(!inPtr){
                //perror(__FUNCTION__);
                return;
            }
            int correct_chars = 0;
            char ch;
            while((ch = fgetc(inPtr)) != EOF && !found){
                if(ch == lookup[correct_chars]){
                    correct_chars++;
                }
                else{
                    correct_chars = 0;
                }
                if(correct_chars == search_width){
                    found = 1;
                }
            }
            if(found){
                //printf("\nFound the phrase: %s\n", lookup);
                strcat(paths_sucessful, path);
                strcat(paths_sucessful, "\n");
            }
            fseek(inPtr, 0, SEEK_SET);
            fclose(inPtr);
            //printf("%s\n", dp->d_name);
            // Construct new path from the base path
            directSearchFilesRecursively(path, paths_sucessful, lookup);
        }
    }

    closedir(dir);
}

void directSpecificSearchFilesRecursively(char *basePath, char* paths_sucessful, char* lookup, char* specific){
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    // Unable to open directory stream
    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL){

        if(strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0 && (EndingEqualsTail(dp->d_name, specific) || dp->d_type == DT_DIR)){

            int found = 0;
            //printf("\nSearching: %s for %s\n", dp->d_name, lookup);
            int search_width = strlen(lookup);

            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);

            FILE* inPtr = fopen(path, "r");
            if(!inPtr){
                //perror(__FUNCTION__);
                return;
            }
            int correct_chars = 0;
            char ch;
            while((ch = fgetc(inPtr)) != EOF && !found){
                if(ch == lookup[correct_chars]){
                    correct_chars++;
                }
                else{
                    correct_chars = 0;
                }
                if(correct_chars == search_width){
                    found = 1;
                }
            }
            if(found){
                //printf("\nFound the phrase: %s\n", lookup);
                strcat(paths_sucessful, path);
                strcat(paths_sucessful, "\n");
            }
            fseek(inPtr, 0, SEEK_SET);
            fclose(inPtr);
            //printf("%s\n", dp->d_name);
            // Construct new path from our base path

            directSpecificSearchFilesRecursively(path, paths_sucessful, lookup, specific);
        }
    }

    closedir(dir);
}
//FOR THE FLAG -f:XYZ WHERE XYZ IS THE "TAIL"
int EndingEqualsTail(char *filename, char* tail){

    if (strlen(tail) > strlen(filename))
        return 0;

    int len = strlen(filename);
    int len2 = strlen(tail);
    
    if(len2 == 1){
        if(filename[len-1] == tail[0]){
            return 1;
        }
    }
    else{
        if (strcmp(&filename[len-strlen(tail)],tail) == 0)
            return 1;
    }
    return 0;
}
// WHEN CHILD FINISHES IT INTERRUPTS THE PRETTY PROMPT AND CLEARS STDIN, REPROMPTS
void siginthandler(int signo){
    if(signo == SIGINT){
        char path[100];
        getcwd(path, 100);
        int stdin_copy = dup(STDIN_FILENO);
        /* remove garbage from stdin */
        tcdrain(stdin_copy);
        tcflush(stdin_copy, TCIFLUSH);
        close(stdin_copy);
        //fflush(stdin); THIS SUCKED
        memset(&input[0], 0, sizeof(input));
        printf("\n\t\tUser input has been interrupted!\n");
        fprintf(stderr, "\nEnter a command of max 100 chars\n");
        fprintf(stderr, "\033[0;34m");
        fprintf(stderr, "findStuff ");
        fprintf(stderr, "%s", path);
        fprintf(stderr, "\033[0m");
        fprintf(stderr, "$ ");
    }
    return;
}
//CREATE THE CHILD MESSAGE FOR LIST, LATER TO BE SENT IN PIPE (IF PIPE EVER WORKS...)
void createChildMessage(int next_cid, char* child_message_in, char *input){
    char sub_message[20];
    strcpy(child_message_in, "\nChild ");
    sprintf(sub_message, "%d", next_cid+1);
    strcat(child_message_in, sub_message);
    strcat(child_message_in, " is looking for ");
    strcat(child_message_in, input);
    strcat(child_message_in, ".\n\0");
    //printf("%s",child_message_in );
}
