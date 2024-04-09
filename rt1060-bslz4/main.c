#include <stdio.h>
#include <stdlib.h>

#include "bitshuffle.h"

#define NX 1028
#define NY 512

int main(int argc, char **argv) {

  if (argc != 4) {
    fprintf(stderr, "%s filename.h5 offset size\n", argv[0]);
    return 1;
  }

  char *chunk = (char *)malloc(atoi(argv[3]));
  uint16_t *buffer = (uint16_t *)malloc(NX * NY * sizeof(uint16_t));

  FILE *fin = fopen(argv[1], "r");

  fseek(fin, atoi(argv[2]), 0);
  fread(chunk, sizeof(char), atoi(argv[3]), fin);
  fclose(fin);

  bshuf_decompress_lz4((chunk) + 12, (void *)buffer, NX * NY, 2, 0);

  fwrite(buffer, sizeof(char), NX * NY * sizeof(uint16_t), stdout);

  free(buffer);
  free(chunk);

  return 0;
}
