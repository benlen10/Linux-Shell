#include <stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<stdbool.h>
#include<ctype.h>

#define BUFFER_SIZE 512
#define SIZE 20

int splitAndExecute(char *input, bool redirectionExists);

void checkForExit(char *response);

void insert(char *temp);

void delet();

void display();

void printError();

int checkForExcl(char *token, bool redirectionExists);

void splitBySpace(char input[512], char *argv[]);

void batchMode(int argc, char *argv[]);

int rear = -1, front = -1;
char *queue[SIZE];
int counter = 1;

void checkForExit(char *response) {

    char exitstr[] = "exit\n";
    char *exitptr = exitstr;
    if (strcmp(response, exitptr) == 0) {
        insert("exit");
        exit(0);
    }
}


int checkForExcl(char *token, bool redirectionExists) {
    /*check for exclamation command */
    //int ret;
    char excl = '!';
    if (token[0] == excl) {
        //TODO check every symbol after ! should be a digit
        int num;
        char *c = token;
        c++;
        while (*c) {
            if (isalpha(*c)) {
                printError();
                return 1;
            }
            //num = atoi(c);//TODO to calculate actual number
            c++;
        }
        c = token;
        c++;
        char *n = (char *) malloc(10 * sizeof(char));
        int i = 0;
        while (isdigit(*c)) {
            n[i] = *c;
            i++;
            c++;
        }
        num = atoi(n);
        //printf("%d=num",num);
        if (strlen(token) == 1) {
            printError();
            return 1;
        }

        int no_of_rec = 0;
        if (front > rear) {
            no_of_rec = 20;
        }
        else {
            no_of_rec = rear - front + 1;
        }


        if (num < counter) {
            printError();
            return 1;
        }
        if (num > (counter + no_of_rec - 1)) {
            printError();
            return 1;
        }

        int index = 0;
        if (front > rear) {
            if ((num - counter) < (SIZE - 1 - front)) {
                index = front + num - counter;
                splitAndExecute(queue[index], redirectionExists);
            }
            else {
                int k = SIZE - 1 - front;
                index = num - counter - k - 1;
                splitAndExecute(queue[index], redirectionExists);

            }
        }
        else {
            index = num - counter;
            splitAndExecute(queue[index], redirectionExists);
        }

        return 1;

    }
    else
        return 0;
    return 0;
}

void printError() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));

}

void insert(char *temp) {
    if ((front == 0 && rear == SIZE - 1) || (front == rear + 1)) {
        counter++;
        delet();
        if (rear == -1) {
            rear = 0;
            front = 0;
        }
        else if (rear == SIZE - 1)
            rear = 0;
        else
            rear++;
        queue[rear] = temp;
    }
    else {
        if (rear == -1) {
            rear = 0;
            front = 0;
        }
        else if (rear == SIZE - 1)
            rear = 0;

        else
            rear++;
        queue[rear] = temp;
    }
}

void delet() {
    char *item = (char *) malloc(10 * sizeof(char));

    if (front == -1)
        return;
    else {
        item = queue[front];

        if (front == rear) {
            front = -1;
            rear = -1;
        }
        else if (front == SIZE - 1)
            front = 0;
        else
            front++;
    }
}

void display() {
    int i, j;
    int temp = counter;
    if (front == 0 && rear == -1) {
        return;
    }
    if (front > rear) {
        for (j = front; j <= SIZE - 1; j++) {
            printf("%d %s\n", temp++, queue[j]);
            //	write(STDOUT_FILENO,"mysh# ",strlen("mysh# ");
        }
        for (i = 0; i <= rear; i++) {
            if (i == rear)
                printf("%d %s", temp++, queue[i]);
            else
                printf("%d %s\n", temp++, queue[i]);
        }
    }
    else {
        for (i = front; i <= rear; i++) {
            if (i == rear)
                printf("\t %d %s", temp++, queue[i]);
            else
                printf("\t %d %s\n", temp++, queue[i]);
        }
    }
    printf("\n");
}

void splitBySpace(char input[512], char *argv[]) {
    int count = 0;
    char *token;
    char delim[2] = " \n";
    token = strtok(input, delim);
    while (token != NULL) {
        argv[count] = strdup(token);
        count++;
        token = strtok(NULL, delim);
    }
    argv[count] = NULL;

}

void batchMode(int argc, char *argv[]) {
    if (argc > 1) {
        if(argc > 2) {
            printError();
            exit(1);
        }
        char *batchFile[2];
        close(STDIN_FILENO);
        FILE *open = fopen(argv[1], "r");
        if (open < 0) {
            printError();
            exit(1);
        }
        char *str;
        size_t size;
        ssize_t chars;
        while ((chars = getline(&str, &size, open)) != -1) {
            write(STDOUT_FILENO, str, strlen(str));
            checkForExit(str);
            if (str[chars - 1] == '\n') {
                str[chars - 1] = '\0';
                --chars;
            }

            splitAndExecute(str, 0);
        }
        exit(0);
    }
}

int splitAndExecute(char *input, bool redirectionExists) {

    /*check for history*/
    char *histIp = strdup(input);
    char *first = strtok(histIp, " \n");
    if (strcmp(first, "history") == 0) {
        if (strtok(NULL, " \n") != NULL) {
            printError();
        }
        else {
            insert("history");
            display();
        }
        return 1;
    }

    /*split based on tokens*/
    char *inputCopy = strdup(input);
    char *argv[512];
    char *redirect_file;
    bool redirection_flag = false;
    char *token;
    char delim[3] = ">\n";
    char delim1[3] = "> \n";
    char *extraRedirect;

    token = strtok(input, delim);//this will store the part before the redirection
    redirect_file = strtok(NULL, delim1);//this will store the part after the redirection symbol
    extraRedirect = strtok(NULL, delim1);//if there are more than 2 files after the redirection

    if (extraRedirect != NULL) {
        printError();
    }
    if (redirectionExists == false) {
        splitBySpace(inputCopy, argv);
    }
    else {
        char delim[4] = " \n";
        char *redirectCopy = redirect_file;
        strtok(redirectCopy, delim);
        if (strtok(NULL, delim) != NULL) {

            printError();
        }
        redirection_flag = true;
        splitBySpace(token, argv);
    }

    /*create new process*/

    insert(input);
    int rc = fork();

    if (rc > 0) {
        int status;
        wait(&status);//TODO check with status
    } else if (rc == 0) {
        if (redirection_flag == true) {
            close(STDOUT_FILENO);
            open(redirect_file, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
        }

        execvp(argv[0], argv);
        printError();
        exit(0);

    } else {
        //fork error
        printError();
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int r;
    for (r = 0; r < SIZE; r++)
        queue[r] = (char *) malloc(20 * sizeof(char));


    batchMode(argc, argv);
    //exit(0);
    while (1) {
        //exit(0);
        write(STDOUT_FILENO, "mysh# ", strlen("mysh# "));

        /*read input given by user*/
        char *input = (char *) malloc(512);
        if (fgets(input, BUFFER_SIZE, stdin) == NULL) {
            printError();
        }

        /*check for exit*/
        checkForExit(input);

        /*check for redirection*/
        bool redirectionExists = false;
        const char *invalid_str = ">";
        char *c = input;
        while (*c) {
            if (strchr(invalid_str, *c)) {
                redirectionExists = true;
            }
            c++;
        }

        /*check for excl*/
        if (checkForExcl(input, redirectionExists) == 1) {
            continue;
        }

        /*split to tokens and execute*/
        if (splitAndExecute(input, redirectionExists)) {
            continue;
        }
    }
}

