#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define READ 0
#define WRITE 1

int main(int argc, char *argv[])
{
    int N;  // the input, the number we all need
    if (argc < 2) {
        perror("Not enough arguments, provide a number");
        exit(EXIT_FAILURE);
    }
    N = atoi(argv[1]);

    // file descriptors for the pipes
    int fd[2];
    pid_t c1, c2; //the children

    // Open and check for errors
    if (pipe(fd) == -1)
    {
        printf("Error opening pipe! \n");
        exit(1);
    }

    c1 = fork();
    if(c1 != 0){
        c2 = fork();
    }

    if(c1 < 0 || c2 <0) {
        perror("Error with children. More common than you'd think");
        exit(EXIT_FAILURE);
    }
    
    int number = 1;
    int read_num;
    int killer = -1;
    while(1)
    {
        if(c1 != 0 && c2 != 0) {
            if (number >= N) {
                write(fd[WRITE], &killer, sizeof(killer));
                write(fd[WRITE], &killer, sizeof(killer));
                break;
            }
            else number++;
            switch(number){
                case 2:printf("2 ");break;
                case 3:printf("3 ");break;
                case 5:printf("5 ");break;
                case 7:printf("7 ");break;
                case 11:printf("11 ");break;
            };
            // printf("This is parent with pid %i \n", getpid());
            // printf("Number is %i \n", number);
            if (!(number%2 == 0 || number%3 == 0 || number%5 == 0 || number%7 == 0 || number%11 == 0))
                write(fd[WRITE], &number, sizeof(number));
        }
        else if(c1 == 0 && c2 != 0) {
            if(read(fd[READ], &read_num, sizeof(read_num))) {
                if(read_num==-1) _exit(0);

                int isprime = 1;
                for(int i = 12;i<read_num;i++) {
                    if(read_num%i==0) isprime = 0;
                }
                
                if (isprime){
                    printf("%i ", read_num);
                    fflush(0);
                }
                // printf("The read number is %i \n", read_num);
            }
            // printf("This is child 1 with pid %i \n", getpid());
        }
        else if(c1 != 0 && c2 == 0) {
            if(read(fd[READ], &read_num, sizeof(read_num))) {
                if(read_num==-1) _exit(0);

                int isprime = 1;
                for(int i = 12;i<read_num;i++) {
                    if(read_num%i==0) isprime = 0;
                }
                
                if (isprime){
                    printf("%i ", read_num);
                    fflush(0);
                }
                // printf("The read number is %i \n", read_num);
            }
            // printf("This is child 2 with pid %i \n", getpid());
        }
    }

    return 0;
}
