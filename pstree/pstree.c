#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdbool.h>
#include <omp.h>

void pstree(bool showpigids) {


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
        pstree(true);
        return 0;
      }
    }
  }
  printf("show pid\n\n");
  pstree(false);
  return 0;
}
