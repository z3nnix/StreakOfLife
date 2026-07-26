#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ALIVE '1'
#define DEAD  '0'
#define DEFAULT_ROWS 30
#define DEFAULT_COLS 50
#define NUM_PATTERNS 18

static int count_neighbors(const char *grid, int rows, int cols, int r, int c) {
	int count = 0;

	for (int dr = -1; dr <= 1; dr++) {
		for (int dc = -1; dc <= 1; dc++) {
			if (dr == 0 && dc == 0)
				continue;
			int nr = (r + dr + rows) % rows;
			int nc = (c + dc + cols) % cols;
			if (grid[nr * cols + nc] == ALIVE)
				count++;
		}
	}
	return count;
}

static void next_gen(const char *cur, char *next, int rows, int cols) {
	for (int r = 0; r < rows; r++) {
		for (int c = 0; c < cols; c++) {
			int n = count_neighbors(cur, rows, cols, r, c);
			if (cur[r * cols + c] == ALIVE)
				next[r * cols + c] = (n == 2 || n == 3) ? ALIVE : DEAD;
			else
				next[r * cols + c] = (n == 3) ? ALIVE : DEAD;
		}
	}
}

static void print_frame(long gen, long party, const char *cur, const char *nxt, int rows, int cols) {
	printf("\n--- party %ld | gen %ld ---\n\n", party, gen);
	for (int r = 0; r < rows; r++) {
		for (int c = 0; c < cols; c++) {
			int idx = r * cols + c;
			if (nxt[idx] == ALIVE) {
				if (cur[idx] == ALIVE)
					fputs("\xe2\x96\x88\xe2\x96\x88", stdout);
				else
					fputs("\xe2\x96\x91\xe2\x96\x91", stdout);
			} else {
				fputs("  ", stdout);
			}
		}
		putchar('\n');
	}
}

static void place_pattern(char *grid, int rows, int cols, int off_r, int off_c, const char **pat, int pat_rows) {
	for (int r = 0; r < pat_rows; r++) {
		int gr = (off_r + r) % rows;
		const char *line = pat[r];
		for (int c = 0; line[c]; c++) {
			int gc = (off_c + c) % cols;
			if (line[c] == '1')
				grid[gr * cols + gc] = ALIVE;
		}
	}
}

typedef struct {
	const char **rows;
	int height;
	const char *name;
} Pattern;

static void generate_random(char *grid, int rows, int cols) {
	memset(grid, DEAD, (size_t)rows * cols);

	const char *g0[] = { "010", "001", "111" };
	const char *g1[] = { "010", "100", "111" };
	const char *g2[] = { "111", "001", "010" };
	const char *g3[] = { "111", "100", "010" };

	const char *pulsar[] = {
		"001110001110", "000000000000", "010001010001",
		"010001010001", "010001010001", "001110001110",
		"000000000000", "001110001110", "010001010001",
		"010001010001", "010001010001", "000000000000",
		"001110001110",
	};

	const char *lwss[] = { "010010", "100001", "100001", "111110" };

	const char *block[] = { "11", "11" };

	const char *blinker[] = { "111" };

	const char *toad[] = { "0111", "1110" };

	const char *beacon[] = { "1100", "1100", "0011", "0011" };

	const char *penta[] = {
		"011100111001110", "100011000110001",
		"000000000000000", "000000000000000",
	};

	const char *rpent[] = { "011", "110", "010" };

	Pattern pool[NUM_PATTERNS] = {
		{ g0,      3, "glider_dr" },
		{ g1,      3, "glider_dl" },
		{ g2,      3, "glider_ur" },
		{ g3,      3, "glider_ul" },
		{ pulsar, 13, "pulsar" },
		{ lwss,    4, "lwss" },
		{ block,   2, "block" },
		{ block,   2, "block" },
		{ block,   2, "block" },
		{ blinker, 1, "blinker" },
		{ blinker, 1, "blinker" },
		{ blinker, 1, "blinker" },
		{ toad,    2, "toad" },
		{ toad,    2, "toad" },
		{ beacon,  4, "beacon" },
		{ penta,   4, "penta" },
		{ rpent,   3, "rpent" },
		{ rpent,   3, "rpent" },
	};

	int placed = 8 + rand() % 5;

	for (int i = 0; i < placed; i++) {
		Pattern *p = &pool[rand() % NUM_PATTERNS];
		int r = rand() % rows;
		int c = rand() % cols;
		place_pattern(grid, rows, cols, r, c, p->rows, p->height);
	}
}

static int read_grid(FILE *fp, char *grid, int rows, int cols) {
	for (int r = 0; r < rows; r++) {
		for (int c = 0; c < cols; c++) {
			int ch = fgetc(fp);
			if (ch == '\n' || ch == '\r')
				ch = fgetc(fp);
			if (ch != ALIVE && ch != DEAD)
				return 0;
			grid[r * cols + c] = (char)ch;
		}
	}
	return 1;
}

static void write_grid(FILE *fp, const char *grid, int rows, int cols) {
	for (int r = 0; r < rows; r++) {
		for (int c = 0; c < cols; c++)
			fputc(grid[r * cols + c], fp);
		fputc('\n', fp);
	}
}

static int load_state(char *cur, char *compare, long *gen, long *party, int *rows, int *cols) {
	FILE *fp = fopen("state.meta", "r");
	if (!fp)
		return 0;

	if (fscanf(fp, "%d %d %ld %ld", rows, cols, gen, party) != 4 ||
	    *rows <= 0 || *cols <= 0) {
		fclose(fp);
		return 0;
	}

	if (!read_grid(fp, cur, *rows, *cols) || !read_grid(fp, compare, *rows, *cols)) {
		fclose(fp);
		return 0;
	}

	fclose(fp);
	return 1;
}

static void save_state(const char *cur, const char *compare, long gen, long party, int rows, int cols) {
	FILE *fp = fopen("state.meta", "w");
	if (!fp)
		return;
	fprintf(fp, "%d %d %ld %ld\n", rows, cols, gen, party);
	write_grid(fp, cur, rows, cols);
	write_grid(fp, compare, rows, cols);
	fclose(fp);
}

int main() {
	srand((unsigned)time(NULL));

	int rows = DEFAULT_ROWS;
	int cols = DEFAULT_COLS;
	long gen = 0;
	long party = 1;

	char *cur     = malloc((size_t)rows * cols);
	char *nxt     = malloc((size_t)rows * cols);
	char *compare = malloc((size_t)rows * cols);
	if (!cur || !nxt || !compare) {
		perror("malloc");
		free(cur);
		free(nxt);
		free(compare);
		return 1;
	}

	if (!load_state(cur, compare, &gen, &party, &rows, &cols)) {
		generate_random(cur, rows, cols);
		memcpy(compare, cur, (size_t)rows * cols);
		gen = 0;
		party = 1;
	}

	next_gen(cur, nxt, rows, cols);

	long new_gen = gen + 1;
	int reset = 0;

	if (new_gen >= 3 && memcmp(nxt, compare, (size_t)rows * cols) == 0)
		reset = 1;

	print_frame(new_gen, party, cur, nxt, rows, cols);

	if (reset) {
		fprintf(stderr, "\n[cycle detected: gen %ld == gen %ld, new party]\n\n",
		        new_gen, new_gen - 2);
		party++;
		generate_random(cur, rows, cols);
		memcpy(compare, cur, (size_t)rows * cols);
		save_state(cur, compare, 0, party, rows, cols);
	} else {
		memmove(compare, cur, (size_t)rows * cols);
		save_state(nxt, compare, new_gen, party, rows, cols);
	}

	free(cur);
	free(nxt);
	free(compare);
	return 0;
}
