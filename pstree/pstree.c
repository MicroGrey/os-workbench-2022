#define _LARGEFILE64_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <linux/types.h>

#define MAX_PROC 55000

typedef struct ProcNode {
  pid_t pid;
  struct ProcNode *next;
} ProcNode;

ProcNode *procchild[MAX_PROC];
char procname[MAX_PROC][256];

void add_child(int parent, int child) {
  ProcNode *node = (ProcNode *)malloc(sizeof(ProcNode));
  node->pid = child;
  node->next = procchild[parent];
  procchild[parent] = node;
}

void print_tree(int pid, int depth,bool printpid) {
  for (int i = 0; i < depth-1; i++) printf("    ");
  printf("|\n");
  for (int i = 0; i < depth-1; i++) printf("    ");
  printf("+-----");
  if (printpid) printf("%s(%d)\n", procname[pid], pid);
  else printf("%s\n", procname[pid]);
  
  for (ProcNode *child = procchild[pid]; child; child = child->next) {
      print_tree(child->pid, depth + 1,printpid);
  }
}

void pstree() {
  int proc_fd = syscall(SYS_open, "/proc", O_DIRECTORY | O_RDONLY);
  if (proc_fd < 0){
    perror("open /proc");
    exit(1);
  }
  char buf[1024];

  struct linux_dirent64 {
      ino64_t d_ino;
      off64_t d_off;
      unsigned short d_reclen;
      unsigned char d_type;
      char d_name[];
  } *d;
  
  int nread;
  while ((nread = syscall(SYS_getdents64, proc_fd, buf, sizeof(buf))) > 0) {
      for (int bpos = 0; bpos < nread; bpos += d->d_reclen) {
          d = (struct linux_dirent64 *)(buf + bpos);

          if (d->d_name[0] < '0' || d->d_name[0] > '9') continue;

          int pid = atoi(d->d_name);
          char path[64];
          sprintf(path, "/proc/%d/status", pid);

          int fd = syscall(SYS_open, path, O_RDONLY);
          if (fd < 0) continue;

          char status[1024];
          int len = syscall(SYS_read, fd, status, sizeof(status) - 1);
          syscall(SYS_close, fd);

          if (len <= 0) continue;
          status[len] = '\0';

          int ppid = 0;
          char name[256] = {0};

          char *line = strtok(status, "\n");
          while (line) {
              if (strncmp(line, "Name:", 5) == 0) {
                  sscanf(line, "Name:\t%s", name);
              } else if (strncmp(line, "PPid:", 5) == 0) {
                  sscanf(line, "PPid:\t%d", &ppid);
              }
              line = strtok(NULL, "\n");
          }

          strcpy(procname[pid], name);
          add_child(ppid, pid);
      }
  }
  syscall(SYS_close, proc_fd);
}

int main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  printf("----------------------------------------\n");
  assert(!argv[argc]);
  int opt;
  while ((opt = getopt(argc, argv, "Vp")) != -1) {
    switch (opt) {
      case 'V': {
        FILE *f = fopen("pstree.txt", "r");
        if (f == NULL) {
          printf("pstree version 1.0\n don't have pstree.txt\n");
          break;
        }
        char line[100];
        while (fgets(line, 100, f) != NULL) {
          printf("%s", line);
        }
        printf("\n\n");
        fclose(f);
        return 0;
      }
      case 'p':{
        printf("show pid\n\n");
        pstree();
        print_tree(1,0,true);
        return 0;
      }
    }
  }
  printf("show pid\n\n");
  pstree();
  print_tree(1,0,false);
  return 0;
}
