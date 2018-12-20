#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define N 10

void code(char *buf, char *key);
void encode(char *buf, char *key);
void after_fork(int file_pipes[2]);
void check_fork(pid_t fork_res);

int main(int argc, char *argv[]) {
	pid_t fork_result;
	int file_pipes1[2];
	int file_pipes2[2];
	char buffer[N + 1];
	char key[N + 1];
	int cnt, status;
	FILE *out;

 	if (!pipe(file_pipes1) && !pipe(file_pipes2)) {
		fork_result = fork();
		check_fork(fork_result);
		if (fork_result == (pid_t)0) {
			close(file_pipes2[1]);
			close(file_pipes2[0]);	
			after_fork(file_pipes1);
			execlp("cat", "cat", argv[1], (char*)0);
			exit(EXIT_FAILURE);	
		} else {
			close(file_pipes1[1]);
			memset(key, 0, N + 1);
			read(file_pipes1[0], key, N);
			close(file_pipes1[0]);
			wait(&status);
			if (!WIFEXITED(status)) {
				printf("Key generator terminated incorrectly!\n");
				exit(EXIT_FAILURE);
			}
			fork_result = fork();
			check_fork(fork_result);
			if (fork_result == (pid_t)0) {				
				after_fork(file_pipes2);
				execlp("cat", "cat", argv[2], (char*)0);
				exit(EXIT_FAILURE);	
			} else {
				close(file_pipes2[1]);
				out = fopen(argv[3], "w");
				if (!out) {
					printf("Error! Unable to create the file!\n");
					fclose(out);
					exit(EXIT_FAILURE);	
				}
				memset(buffer, 0, N);
				for (; cnt = read(file_pipes2[0], buffer, N);) {
					if (argv[4]) {
						code(buffer, key);
					} else {
						encode(buffer, key);
					}
					if (fputs(buffer, out) == EOF) {
						printf("Error! Unable to write in file!\n");
						fclose(out);
						exit(EXIT_FAILURE);
					}
					memset(buffer, 0, N);
				}
				fclose(out);
			}	
	  	}
 	}
	printf("Success!\n");
	exit(EXIT_SUCCESS);
}

void after_fork(int file_pipes[2])
{
	close(1);
	dup(file_pipes[1]);
	close(file_pipes[1]);
	close(file_pipes[0]);
}

void code(char *buf, char *key) {
	int i;
	for (i = 0; i < N; ++i) {
		buf[i] += key[i];	
	} 
}

void encode(char *buf, char *key) {
	int i;
	for (i = 0; i < N; ++i) {
		buf[i] -= key[i];	
	} 
}

void check_fork(pid_t fork_res) {
	if (fork_res == (pid_t)-1) {
		fprintf(stderr, "Fork failure");
		exit(EXIT_FAILURE);
	}
}
