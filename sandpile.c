#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

typedef struct {
  bool original;
  bool o1;
  bool o2;
  bool o3;
  bool o4;
  bool o5;
  bool o6;
} Flags;
Flags optimizations = {0};

static void die(const char *s) {
  fprintf(stderr, "%s\n", s);
  exit(1);
}

int N = 801;
int **grid;

// you can also use this function to debug print your sandpile
static void render(char *filename) {
  int chan = 3;
  unsigned char *pixels = (unsigned char *)malloc(N * N * chan * sizeof(char));
  int stride = N * chan;
  if (!pixels)
    die("malloc failed for PNG pixel buffer");
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      unsigned char r = 0, g = 0, b = 0;
      switch (grid[i][j]) {
      case 0: // obsidian
        r = 15;
        g = 15;
        b = 25;
        break;
      case 1: // electric cyan
        r = 0;
        g = 210;
        b = 255;
        break;
      case 2: // vibrant magenta
        r = 230;
        g = 30;
        b = 130;
        break;
      case 3: // bright gold
        r = 255;
        g = 200;
        b = 0;
        break;
      default: // white
        r = 0;
        g = 0;
        b = 0;
      }

      int pid = (i * N + j) * chan;
      pixels[pid] = r;
      pixels[pid + 1] = g;
      pixels[pid + 2] = b;
    }
  }
  if (stbi_write_png(filename, N, N, chan, pixels, stride)) {
    printf("Successfully saved sandpile identity to: %s\n", filename);
  } else {
    die("Error: stb_image_write failed to save png file\n");
  }
  free(pixels);
}

static int *left;
static int *right;
static void bounds() {
  left = (int *)malloc(N * sizeof(int));
  right = (int *)malloc(N * sizeof(int));
  for (int i = 0; i < N; ++i) {
    left[i] = -1;
    for (int j = 0; j <= MAX(i, N - 1 - i); ++j) {
      if ((i - N / 2) * (i - N / 2) + (j - N / 2) * (j - N / 2) <=
          (N / 2) * (N / 2)) {
        if (left[i] == -1) {
          left[i] = j;
        }
        right[i] = j;
      }
    }
  }
}

static void stabilize() {
  int changed = 0;
  if (optimizations.original) {
    do {
      changed = 0;
      for (int i = 0; i < N; ++i) {
        for (int j = 0; j <= MAX(i, N - 1 - i); ++j) {
          if ((i - N / 2) * (i - N / 2) + (j - N / 2) * (j - N / 2) <=
              (N / 2) * (N / 2)) {
            if (grid[i][j] < 4)
              continue;
            changed = 1; // topple it
            grid[i][j] -= 4;
            if (i > 0)
              grid[i - 1][j]++;
            if (j > 0)
              grid[i][j - 1]++;
            if (i < N - 1)
              grid[i + 1][j]++;
            if (j < N - 1)
              grid[i][j + 1]++;
          }
        }
      }
    } while (changed);
  } else if (optimizations.o3) {
    do {
      changed = 0;
      for (int phase = 0; phase < 3; ++phase) {
        #pragma omp parallel for reduction(|:changed)
        for (int i = phase; i < N; i += 3) {
          for (int j = left[i]; j <= right[i]; ++j) {
            if (grid[i][j] < 4)
              continue;
            changed = 1;
            int topples = grid[i][j] / 4;
            grid[i][j] %= 4;
            if (i > 0)
              grid[i - 1][j] += topples;
            if (j > 0)
              grid[i][j - 1] += topples;
            if (i < N - 1)
              grid[i + 1][j] += topples;
            if (j < N - 1)
              grid[i][j + 1] += topples;
          }
        }
      }
    } while (changed);
  } else if (optimizations.o2) {
    do {
      changed = 0;
      for (int i = 0; i < N; ++i) {
        for (int j = left[i]; j <= right[i]; ++j) {
          if (grid[i][j] < 4)
            continue;
          changed = 1;
          int topples = grid[i][j] / 4;
          grid[i][j] %= 4;
          if (i > 0)
            grid[i - 1][j] += topples;
          if (j > 0)
            grid[i][j - 1] += topples;
          if (i < N - 1)
            grid[i + 1][j] += topples;
          if (j < N - 1)
            grid[i][j + 1] += topples;
        }
      }
    } while (changed);
  } else if (optimizations.o1) {
    do {
      changed = 0;
      for (int i = 0; i < N; ++i) {
        for (int j = left[i]; j <= right[i]; ++j) {
          if (grid[i][j] < 4)
            continue;
          changed = 1;
          grid[i][j] -= 4;
          if (i > 0)
            grid[i - 1][j]++;
          if (j > 0)
            grid[i][j - 1]++;
          if (i < N - 1)
            grid[i + 1][j]++;
          if (j < N - 1)
            grid[i][j + 1]++;
        }
      }
    } while (changed);
  }
}

void find_identity() {
  if (optimizations.o1 || optimizations.o2 || optimizations.o3) {
    bounds();
  }
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      grid[i][j] = 6;
    }
  }
  stabilize();
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      grid[i][j] = 6 - grid[i][j];
    }
  }
  stabilize();
  if (left != NULL || right != NULL) {
    free(left);
    free(right);
  }
}

void parse_optimization(const char *optimization) {
  if (strcmp(optimization, "all") == 0) {
    optimizations.o1 = optimizations.o2 = optimizations.o3 = optimizations.o4 = optimizations.o5 = optimizations.o6 = true;
    return;
  }
  char *copy = strdup(optimization);
  char *token = strtok(copy, "+");
  while (token != NULL) {
    if (strcmp(token, "o1") == 0) optimizations.o1 = true;
    else if (strcmp(token, "o2") == 0) optimizations.o2 = true;
    else if (strcmp(token, "o3") == 0) optimizations.o3 = true;
    else if (strcmp(token, "o4") == 0) optimizations.o4 = true;
    else if (strcmp(token, "o5") == 0) optimizations.o5 = true;
    else if (strcmp(token, "o6") == 0) optimizations.o6 = true;
    else {
      free(copy);
      die("Unknown optimization flag");
    }
    token = strtok(NULL, "+");
  }
  free(copy);
}

int main(int argc, char **argv) {
  if (argc < 2 || argc > 3) {
    fprintf(stderr, "Usage: %s N [optimization]\n", argv[0]);
    return EXIT_FAILURE;
  }
  char *end;
  uintmax_t parsed = strtoumax(argv[1], &end, 10);
  if (errno || argv[1][0] < '0' || argv[1][0] > '9' || *end || !parsed ||
      parsed > INT32_MAX)
    die("grid size must be a positive int");
  N = (int)parsed;
  if (argc == 3) {
    parse_optimization(argv[2]);
  } else {
    optimizations.original = true;
  }

  grid = (int **)malloc(N * sizeof(int *));
  for (int i = 0; i < N; ++i) {
    grid[i] = (int *)malloc(N * sizeof(int));
  }

  printf("Running test\n");
  struct timeval start, stop;
  gettimeofday(&start, NULL);
  find_identity();
  gettimeofday(&stop, NULL);
  float elapsed = (stop.tv_sec - start.tv_sec) + 1e-6 * (stop.tv_usec - start.tv_usec);
  printf("Time for size %d: %0.6f seconds\n", N, elapsed);

  render("identity.png");
  for (int i = 0; i < N; ++i) {
    free(grid[i]);
  }
  free(grid);
  return EXIT_SUCCESS;
}
