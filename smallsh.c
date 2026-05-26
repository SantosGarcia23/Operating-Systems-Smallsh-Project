#define _GNU_SOURCE
#include <stdio.h> 
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <unistd.h> 
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


#include <signal.h>

#define INPUT_LENGTH 2048
#define MAX_ARGS 512


int status = 0;
bool fg_mode = false;

/* Our signal handler for SIGINT */
void handle_SIGINT(int signo){
  char* message = "Terminated by signal 2\n";
  // We are using write rather than printf
  write(STDOUT_FILENO, message, 24);
}

void handle_SIGSTP(int signo) {
    if (!fg_mode) {
        char* message = "\nEntering foreground-only mode (& is now ignored)\n";
        write(STDOUT_FILENO, message, 50);
        fg_mode = true;
    } else {
        char* message = "\nExiting foreground-only mode\n";
        write(STDOUT_FILENO, message, 30);
        fg_mode = false;
    }
}


struct command_line{
    char *argv[MAX_ARGS + 1];
    int argc;
    char *input_file;
    char *output_file;
    bool is_bg;
};

struct command_line *parse_input(){
    char input[INPUT_LENGTH];
    struct command_line *curr_command = (struct command_line *) calloc(1, sizeof(struct command_line));

    // Get input
    printf(": ");
    fflush(stdout);
    fgets(input, INPUT_LENGTH, stdin);

    // Tokenize the input
    char *token = strtok(input, " \n");

    while(token){
      if(!strcmp(token,"<")){
        curr_command->input_file = strdup(strtok(NULL," \n"));
      } else if(!strcmp(token,">")){
          curr_command->output_file = strdup(strtok(NULL," \n"));
      } else if(!strcmp(token,"&")){
          curr_command->is_bg = true;
      } else{
          curr_command->argv[curr_command->argc++] = strdup(token);
      }

      token=strtok(NULL," \n");
    }

    return curr_command;
}

int main(){
    struct command_line *curr_command;


    // Initialize SIGINT_action struct to be empty
    struct sigaction SIGINT_action = {0}, ignore_action = {0};

    // Fill out the SIGINT_action struct
    // Register handle_SIGINT as the signal handler
    SIGINT_action.sa_handler = handle_SIGINT;
    // Block all catchable signals while handle_SIGINT is running
    sigfillset(&SIGINT_action.sa_mask);
    // No flags set
    SIGINT_action.sa_flags = 0;

    // Install our signal handler
    sigaction(SIGINT, &SIGINT_action, NULL);

    // The ignore_action struct as SIG_IGN as its signal handler
    ignore_action.sa_handler = SIG_IGN;




    // Initialize SIGSTP_action struct to be empty
    struct sigaction SIGSTP_action = {0};

    // Fill out the SIGSTP_action struct
    // Register handle_SIGSTP as the signal handler
    SIGSTP_action.sa_handler = handle_SIGSTP;
    // Block all catchable signals while handle_SIGSTP is running
    sigfillset(&SIGSTP_action.sa_mask);
    // No flags set
    SIGSTP_action.sa_flags = 0;

    // Install our signal handler
    sigaction(SIGTSTP, &SIGSTP_action, NULL);

    // The ignore_action struct as SIG_IGN as its signal handler
    ignore_action.sa_handler = SIG_IGN;

  
    while(true){

        int bg_status;
        pid_t bg_pid;
        while ((bg_pid = waitpid(-1, &bg_status, WNOHANG)) > 0) {
            if (WIFEXITED(bg_status)) {
                printf("background pid %d is done: exit value %d\n",
                    bg_pid, WEXITSTATUS(bg_status));
            } else if (WIFSIGNALED(bg_status)) {
                printf("background pid %d is done: terminated by signal %d\n",
                    bg_pid, WTERMSIG(bg_status));
            }
            fflush(stdout);
        }


        curr_command = parse_input();

        if (curr_command == NULL || curr_command->argc == 0) {
            continue;
        }

        if (curr_command->argv[0][0] == '#') {
            continue;
        }

        if (strcmp(curr_command->argv[0], "status") == 0) {
            if (WIFEXITED(status)) {
                printf("exit value %d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("terminated by signal %d\n", WTERMSIG(status));
            }
            fflush(stdout);
            continue;
        }

        if (strcmp(curr_command->argv[0], "exit") == 0) {
            exit(0);
        }   

        if (strcmp(curr_command->argv[0], "cd") == 0) {
            char *path;

            if (curr_command->argc > 1) {
                path = curr_command->argv[1];
            } else {
                path = getenv("HOME");
            }

            if (chdir(path) == -1) {
                perror("cd");
            }
            continue;
        }

        pid_t pid = fork();

        if (pid == 0) {

            // Initialize SIGINT_action struct to be empty
            struct sigaction SIGINT_action = {0}, ignore_action = {0};

            // Fill out the SIGINT_action struct
            // Register handle_SIGINT as the signal handler
            SIGINT_action.sa_handler = handle_SIGINT;
            // Block all catchable signals while handle_SIGINT is running
            sigfillset(&SIGINT_action.sa_mask);
            // No flags set
            SIGINT_action.sa_flags = 0;

            // Install our signal handler
            sigaction(SIGINT, &SIGINT_action, NULL);

            // The ignore_action struct as SIG_DFL as its signal handler
            ignore_action.sa_handler = SIG_DFL;





            // Initialize SIGSTP_action struct to be empty
            struct sigaction SIGSTP_action = {0};

            // Fill out the SIGSTP_action struct
            // Register handle_SIGSTP as the signal handler
            SIGSTP_action.sa_handler = handle_SIGSTP;
            // Block all catchable signals while handle_SIGSTP is running
            sigfillset(&SIGSTP_action.sa_mask);
            // No flags set
            SIGSTP_action.sa_flags = 0;

            // Install our signal handler
            sigaction(SIGTSTP, &SIGSTP_action, NULL);

            // The ignore_action struct as SIG_DFL as its signal handler
            ignore_action.sa_handler = SIG_DFL;


            if (curr_command->input_file != NULL) {
                int targetFD = open(curr_command->input_file, O_RDONLY);
                
                if (targetFD == -1) {
                    perror("open()");   
                    exit(1);
                }
                if (dup2(targetFD, 0) == -1) {
                    perror("dup2");
                    exit(1);
                }
                close(targetFD);
                }
            

                if (curr_command->is_bg && !fg_mode && curr_command->input_file == NULL) {
                    int targetFD = open("/dev/null", O_RDONLY);
                    if (targetFD == -1) {
                    perror("open()");
                    exit(1);
                    }
                    if (dup2(targetFD, 0) == -1) {
                        perror("dup2");
                        exit(1);
                    }
                    close(targetFD);
                }


            if (curr_command->output_file != NULL){
                int targetFD = open(curr_command->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0640);
                if (targetFD == -1) {
                    perror("open()");
                    exit(1);
                }

                // Use dup2 to point FD 1, i.e., standard output to targetFD
                int result = dup2(targetFD, 1);
                if (result == -1) {
                    perror("dup2"); 
                    exit(1); 
                } 

                close(targetFD);
            }


            if (curr_command->is_bg && !fg_mode && curr_command->output_file == NULL) {
                int targetFD = open("/dev/null", O_WRONLY);
                if (targetFD == -1) {
                    perror("open()");
                    exit(1);
                }
                if (dup2(targetFD, 0) == -1) {
                    perror("dup2");
                    exit(1);
                }
                close(targetFD);
            }

            execvp(curr_command->argv[0], curr_command->argv);
            perror(curr_command->argv[0]);
            exit(1);

        } 
        int child_status;
        if (curr_command->is_bg) {
            
            printf("background pid is %d\n", pid);
            fflush(stdout);
        } else {
            
            waitpid(pid, &child_status, 0);
            status = child_status;

            if (WIFSIGNALED(child_status)) {
                printf("terminated by signal %d\n", WTERMSIG(child_status));
                fflush(stdout);
            }
        }                
    } 
}
