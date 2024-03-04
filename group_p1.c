#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_CMD_LEN 256
#define MAX_TOKENS 64
// Function declarations
void execute_internal_command(char *tokens[]);
void execute_external_command(char *tokens[]);

// Rest of your code

char path_variable[MAX_CMD_LEN] = "/bin:/sbin";  // Initial path variable
// Function definition
void print_path() {
    // Print the current pathname variable
    printf("Current Path: %s\n", path_variable);
}
void parse_command(char *buf, char *tokens[]) {
    char *token;
    int token_count = 0;

    // Tokenizing the input command
    token = strtok(buf, " \n");
    while (token != NULL && token_count < MAX_TOKENS - 1) {
        if (token_count == 0 && token[0] == '+') {
            // Handle addition separately
            char *addition_token = strtok(NULL, " \n");
            if (addition_token != NULL) {
                // Prepend path_variable to the addition token
                char *new_path = malloc(strlen(path_variable) + strlen(addition_token) + 2);
                if (new_path != NULL) {
                    strcpy(new_path, path_variable);
                    strcat(new_path, ":");
                    strcat(new_path, addition_token);

                    // Free the old path and update path_variable
                    strcpy(path_variable, new_path);

                    free(new_path);  // Free the temporary new_path
                    printf("New Path: %s\n", path_variable);
                } else {
                    fprintf(stderr, "Memory allocation error\n");
                    exit(EXIT_FAILURE);
                }
            } else {
                fprintf(stderr, "Invalid path command: +%s\n", addition_token);
            }
        } else {
            // Regular tokenization
            tokens[token_count++] = token;
        }

        token = strtok(NULL, " \n");
    }

    tokens[token_count] = NULL;  // Setting the last element to NULL
}





void execute_internal_command(char *tokens[]) {
    if (strcmp(tokens[0], "cd") == 0) {
        if (tokens[1] == NULL) {
            fprintf(stderr, "cd: missing argument\n");
        } else {
            printf("Attempting to change directory to: %s\n", tokens[1]);
            if (chdir(tokens[1]) != 0) {
                perror("cd");
            }
        }
    } else if (strcmp(tokens[0], "path") == 0) {
        modify_path(tokens);
        // Handle path command
    } else if (strcmp(tokens[0], "quit") == 0) {
      exit(0);  // Quit the shell
    } else if (strcmp(tokens[0], "pwd") == 0) {
        char cwd[256];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("getcwd");
        }
    } else if (strcmp(tokens[0], "ls") == 0) {
        // Handle ls command
    } else {
        // External command handling
        execute_external_command(tokens);
    }
}


// Previous code...

void modify_path(char *tokens[]) {
    printf("Tokens: ");
    for (int i = 0; tokens[i] != NULL; ++i) {
        printf("%s ", tokens[i]);
    }
    printf("\n");

    if (tokens[1] == NULL) {
        // Print current path if no argument is provided
        print_path();
    } else if (tokens[1][0] == '+' && tokens[2] != NULL) {
        // Append a directory to the path
        char *new_path = malloc(strlen(path_variable) + strlen(tokens[2]) + 2); // +2 for ':' and null terminator
        if (new_path != NULL) {
            strcpy(new_path, path_variable);
            strcat(new_path, ":");
            strcat(new_path, tokens[2]);

            // Free the old path and update path_variable
            strcpy(path_variable, new_path);

            free(new_path);  // Free the temporary new_path
            printf("New Path: %s\n", path_variable);
        } else {
            fprintf(stderr, "Memory allocation error\n");
            exit(EXIT_FAILURE);
        }
    } else if (tokens[1][0] == '/' && tokens[2] != NULL && tokens[3] == NULL) {
        // Absolute path provided, replace the current path
        strcpy(path_variable, tokens[2]);
        printf("New Path: %s\n", path_variable);
    } else if (tokens[1][0] == '-' && tokens[2] != NULL) {
        // Remove a directory from the path
        char *dirToRemove = tokens[2];
        char *pos = strstr(path_variable, dirToRemove);
        if (pos != NULL) {
            // Directory found, remove it
            memmove(pos, pos + strlen(dirToRemove), 1 + strlen(pos + strlen(dirToRemove)));
            printf("New Path: %s\n", path_variable);
        } else {
            fprintf(stderr, "Directory not found in the path\n");
        }
    } else {
        // Invalid usage, print an error message
        fprintf(stderr, "Invalid path command: %s %s %s\n", tokens[0], tokens[1], (tokens[2] != NULL) ? tokens[2] : "");
    }
}

// Rest of your code...



void execute_external_command(char *tokens[]) {
    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        execv(tokens[0], tokens);
        // If execv fails, print an error message
        perror("Command not found");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("Fork failed");
    }
}

int main() {
    char buf[MAX_CMD_LEN];
    char *tokens[MAX_TOKENS];

    while (1) {
        printf("shell> ");
        fgets(buf, MAX_CMD_LEN, stdin);

        // Remove the newline character at the end
        buf[strcspn(buf, "\n")] = '\0';

        parse_command(buf, tokens);

        if (tokens[0] != NULL) {
            if (strcmp(tokens[0], "path") == 0 || strcmp(tokens[0], "quit") == 0  || strcmp(tokens[0], "cd") == 0 || strcmp(tokens[0], "pwd") == 0 ) {
                // If the command is 'path', handle it internally
                execute_internal_command(tokens);
            } else {
                // If it's not an internal command, execute it externally
                execute_external_command(tokens);
            }
        }
    }

    return 0;
}

