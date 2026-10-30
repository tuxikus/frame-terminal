#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <util.h>
#include <string.h>

int main() {
  int master_fd; // pty -> zsh
  pid_t pid = forkpty(&master_fd, NULL, NULL, NULL);
  printf("my_pid: %ld | forkpty: %ld\n",(long)getpid(), (long)pid);

  if (pid == 0) {
    execlp("zsh", "zsh", NULL);
    perror("execlp");
    exit(1);
  }
  
  sleep(1);
  /* write(master_fd, "echo hello from C\n", strlen("echo hello from C\n")); */
  write(master_fd, "ls -lah\n", strlen("ls -lah\n"));

  char buffer[4096];
  while (1) {
    ssize_t n = read(master_fd, buffer, sizeof(buffer) - 1);

    if (n <= 0)
      break;

    buffer[n] = '\0';
    printf("%s", buffer);
    fflush(stdout);
  }
  
  printf("exit...\n");
 
  return 0;
}
