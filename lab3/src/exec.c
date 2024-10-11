#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main( void ) {
    char *app = "./sequential_min_max";

	char *app_args[] = {app, "1", "10", NULL};

	pid_t pid = fork();

	if (pid == 0) {
        execvp(app, app_args);
        perror("execvp failed");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0); 
        if (WIFEXITED(status)) {
            printf("sequential_min_max завершился с кодом: %d\n", WEXITSTATUS(status));
        } else {
            printf("sequential_min_max завершился с ошибкой\n");
        }
    }

	return 0;
}