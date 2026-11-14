/*
 * MiniSPICE - a tiny DC circuit solver
 *
 * Solves resistor networks driven by one DC voltage source using
 * nodal analysis (KCL) and Gaussian elimination with partial pivoting.
 *
 * Build:  gcc minispice.c -o minispice -lm
 * Run:    ./minispice
 */

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NODES    10      /* including ground (node 0)            */
#define MAX_COMP     20      /* resistors                            */
#define PIVOT_EPS    1e-12   /* smaller pivot => singular matrix     */
#define LINE_LEN     128
#define RESULTS_FILE "results.txt"
#define RULE         "  ------------------------------------------------------------------\n"

/* ------------------------------------------------------------------ */
/*  Data                                                              */
/* ------------------------------------------------------------------ */

typedef struct {
    int    n1, n2;
    double ohms;
} Resistor;

typedef struct {
    int      nodes;                      /* node count, ground included     */
    double   g[MAX_NODES][MAX_NODES];    /* total conductance per node pair */
    Resistor parts[MAX_COMP];
    int      count;
    int      src_node;                   /* + terminal, 0 = not set yet     */
    double   src_volts;                  /* - terminal is always ground     */
} Circuit;

typedef struct {
    int    ready;                        /* 1 = matches the current circuit */
    double v[MAX_NODES];                 /* node voltages                   */
    double i[MAX_COMP];                  /* current n1 -> n2 per resistor   */
    double p[MAX_COMP];                  /* power per resistor              */
    double src_current;
    double total_power;
} Solution;

/* ------------------------------------------------------------------ */
/*  Input helpers: whole lines only, so "abc" or "10x" never slip in  */
/* ------------------------------------------------------------------ */

static void read_line(char *buf)
{
    if (fgets(buf, LINE_LEN, stdin) == NULL) {
        printf("\n\n  Input closed. Goodbye!\n");
        exit(0);
    }
    if (strchr(buf, '\n') == NULL) {     /* line too long: drop the rest */
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
    }
}

static int only_spaces(const char *s)
{
    while (isspace((unsigned char)*s))
        s++;
    return *s == '\0';
}

static int read_int(const char *prompt, int lo, int hi)
{
    char buf[LINE_LEN], *end;

    for (;;) {
        printf("%s", prompt);
        read_line(buf);
        long x = strtol(buf, &end, 10);
        if (end != buf && only_spaces(end) && x >= lo && x <= hi)
            return (int)x;
        printf("  ! Enter a whole number from %d to %d.\n", lo, hi);
    }
}

static double read_double(const char *prompt, int non_negative)
{
    char buf[LINE_LEN], *end;

    for (;;) {
        printf("%s", prompt);
        read_line(buf);
        double x = strtod(buf, &end);
        if (end != buf && only_spaces(end) && isfinite(x) && !(non_negative && x < 0))
            return x;
        printf(non_negative ? "  ! Enter a number >= 0.\n" : "  ! Enter a number.\n");
    }
}

/* Formats a value with an SI prefix: 0.0333 A -> "33.3333 mA" */
static const char *si(double x, const char *unit, char *buf, size_t n)
{
    static const struct { double scale; const char *prefix; } p[] = {
        { 1e3, "k" }, { 1.0, "" }, { 1e-3, "m" }, { 1e-6, "u" }
    };
    double a = fabs(x);
    int k = 0;

    if (a == 0.0) {
        snprintf(buf, n, "%9.4f %s", 0.0, unit);
        return buf;
    }
    while (k < 3 && a < p[k].scale)
        k++;
    snprintf(buf, n, "%9.4f %s%s", x / p[k].scale, p[k].prefix, unit);
    return buf;
}

/* ------------------------------------------------------------------ */
/*  Screens                                                           */
/* ------------------------------------------------------------------ */

static void banner(void)
{
    printf("\n");
    printf("  +==============================================================+\n");
    printf("  |                         MiniSPICE                            |\n");
    printf("  |        DC circuit solver  -  nodal analysis + Gauss          |\n");
    printf("  +==============================================================+\n\n");
    printf("  Ground is node 0. Up to %d nodes and %d resistors.\n", MAX_NODES, MAX_COMP);
    printf("  One voltage source, negative terminal on ground.\n\n");
}

static void print_menu(const Circuit *c, const Solution *s)
{
    printf("\n  +--------------------------- MENU -----------------------------+\n");
    printf("  |  1  Set resistors                                            |\n");
    printf("  |  2  Set voltage source                                       |\n");
    printf("  |  3  Solve                                                    |\n");
    printf("  |  4  View results                                             |\n");
    printf("  |  5  Save results to %-40s |\n", RESULTS_FILE);
    printf("  |  6  Exit                                                     |\n");
    printf("  +--------------------------------------------------------------+\n");
    printf("  Nodes: %d   Resistors: %d   Source: ", c->nodes, c->count);
    if (c->src_node)
        printf("%.4f V on node %d", c->src_volts, c->src_node);
    else
        printf("not set");
    printf("   Status: %s\n\n", s->ready ? "solved" : "not solved");
}

/* ------------------------------------------------------------------ */
/*  Circuit entry                                                     */
/* ------------------------------------------------------------------ */

static void input_resistors(Circuit *c)
{
    char prompt[LINE_LEN];

    memset(c->g, 0, sizeof c->g);
    c->count = 0;

    printf("\n  RESISTORS\n" RULE);
    printf("  Enter ohms for each node pair, 0 if nothing is connected.\n\n");

    for (int a = 0; a < c->nodes; a++) {
        for (int b = a + 1; b < c->nodes; b++) {
            int last_pair = (a == c->nodes - 2 && b == c->nodes - 1);

            for (;;) {
                snprintf(prompt, sizeof prompt, "  N%d - N%d  : ", a, b);
                double r = read_double(prompt, 1);
                if (r == 0.0)
                    break;

                if (c->count == MAX_COMP) {
                    printf("  ! Limit of %d resistors reached, value ignored.\n", MAX_COMP);
                    break;
                }

                /* parallel resistors on one pair: conductances add up */
                c->g[a][b] += 1.0 / r;
                c->g[b][a] = c->g[a][b];
                c->parts[c->count++] = (Resistor){ a, b, r };

                int more = read_int(last_pair
                                        ? "           1 = add in parallel, 2 = done      : "
                                        : "           1 = add in parallel, 2 = next pair : ",
                                    1, 2);
                if (more == 2)
                    break;
            }
        }
    }
    printf("\n  %d resistor(s) stored.\n", c->count);
}

static void input_source(Circuit *c)
{
    char prompt[LINE_LEN];

    printf("\n  VOLTAGE SOURCE\n" RULE);
    printf("  Negative terminal is on ground (node 0).\n\n");

    snprintf(prompt, sizeof prompt, "  Positive terminal node [1-%d] : ", c->nodes - 1);
    c->src_node = read_int(prompt, 1, c->nodes - 1);
    c->src_volts = read_double("  Voltage (V)                  : ", 0);
}

/* ------------------------------------------------------------------ */
/*  Solver                                                            */
/* ------------------------------------------------------------------ */

/*
 * Builds the augmented system [G | I] for nodes 1..n-1 (ground is left out).
 * Row k-1 is KCL at node k, except the source node, whose row is V = volts.
 */
static int build_system(const Circuit *c, double m[][MAX_NODES])
{
    int n = c->nodes - 1;

    for (int r = 0; r < n; r++)
        for (int col = 0; col <= n; col++)
            m[r][col] = 0.0;

    for (int k = 1; k <= n; k++) {
        double *row = m[k - 1];

        if (k == c->src_node) {
            row[k - 1] = 1.0;
            row[n] = c->src_volts;
            continue;
        }
        for (int j = 0; j < c->nodes; j++) {
            if (j == k || c->g[k][j] == 0.0)
                continue;
            row[k - 1] += c->g[k][j];          /* conductances leaving node k */
            if (j != 0)
                row[j - 1] -= c->g[k][j];      /* coupling to neighbour j     */
        }
    }
    return n;
}

static void print_system(double m[][MAX_NODES], int n)
{
    printf("\n  NODAL SYSTEM   [ G (siemens) | I ]   rows/cols = nodes 1..%d\n" RULE, n);
    for (int r = 0; r < n; r++) {
        printf("  ");
        for (int col = 0; col < n; col++)
            printf("%9.5f ", m[r][col]);
        printf("|%10.4f\n", m[r][n]);
    }
}

/* Gaussian elimination with partial pivoting. Returns 0 if singular. */
static int gauss(double m[][MAX_NODES], int n, double *x)
{
    for (int col = 0; col < n; col++) {
        int pivot = col;
        for (int r = col + 1; r < n; r++)
            if (fabs(m[r][col]) > fabs(m[pivot][col]))
                pivot = r;

        if (fabs(m[pivot][col]) < PIVOT_EPS)
            return 0;

        if (pivot != col)
            for (int k = 0; k <= n; k++) {
                double t = m[col][k];
                m[col][k] = m[pivot][k];
                m[pivot][k] = t;
            }

        for (int r = col + 1; r < n; r++) {
            double f = m[r][col] / m[col][col];
            for (int k = col; k <= n; k++)
                m[r][k] -= f * m[col][k];
        }
    }

    for (int r = n - 1; r >= 0; r--) {
        double sum = m[r][n];
        for (int k = r + 1; k < n; k++)
            sum -= m[r][k] * x[k];
        x[r] = sum / m[r][r];
    }
    return 1;
}

static void solve(const Circuit *c, Solution *s)
{
    double m[MAX_NODES][MAX_NODES];      /* n <= 9 rows, n + 1 <= 10 columns */
    double x[MAX_NODES];

    s->ready = 0;
    if (c->src_node == 0) {
        printf("\n  ! Set the voltage source first (option 2).\n");
        return;
    }

    int n = build_system(c, m);
    print_system(m, n);

    if (!gauss(m, n, x)) {
        printf("\n  ! Matrix is singular: some node has no path to ground.\n");
        return;
    }

    s->v[0] = 0.0;
    for (int k = 1; k <= n; k++)
        s->v[k] = x[k - 1];

    s->src_current = 0.0;
    s->total_power = 0.0;
    for (int k = 0; k < c->count; k++) {
        const Resistor *r = &c->parts[k];

        s->i[k] = (s->v[r->n1] - s->v[r->n2]) / r->ohms;
        s->p[k] = s->i[k] * s->i[k] * r->ohms;
        s->total_power += s->p[k];

        /* KCL at the source node: everything leaving it comes from the source */
        if (r->n1 == c->src_node)
            s->src_current += s->i[k];
        else if (r->n2 == c->src_node)
            s->src_current -= s->i[k];
    }

    s->ready = 1;
    printf("\n  Solved. Choose 4 to view the results.\n");
}

/* ------------------------------------------------------------------ */
/*  Output (same report for the screen and the file)                  */
/* ------------------------------------------------------------------ */

static void print_results(FILE *out, const Circuit *c, const Solution *s)
{
    char b1[32], b2[32], b3[32];

    fprintf(out, "\n  NODE VOLTAGES\n" RULE);
    for (int k = 0; k < c->nodes; k++)
        fprintf(out, "  N%-2d %s%s\n", k, si(s->v[k], "V", b1, sizeof b1),
                k == 0 ? "      (ground)" : "");

    fprintf(out, "\n  RESISTORS\n" RULE);
    fprintf(out, "  %-5s %-11s %-14s %-14s %s\n", "ID", "Flow", "  Resistance", "  Current", "  Power");
    fprintf(out, RULE);
    for (int k = 0; k < c->count; k++) {
        const Resistor *r = &c->parts[k];
        int from = r->n1, to = r->n2;
        char flow[16];

        if (s->i[k] < 0) {                 /* show the real direction of flow */
            from = r->n2;
            to = r->n1;
        }
        snprintf(flow, sizeof flow, "N%d -> N%d", from, to);
        fprintf(out, "  R%-4d %-11s %-14s %-14s %s\n", k + 1, flow,
                si(r->ohms, "ohm", b1, sizeof b1),
                si(fabs(s->i[k]), "A", b2, sizeof b2),
                si(s->p[k], "W", b3, sizeof b3));
    }

    fprintf(out, "\n  SOURCE\n" RULE);
    fprintf(out, "  Voltage         %-14s   (node %d to ground)\n",
            si(c->src_volts, "V", b1, sizeof b1), c->src_node);
    fprintf(out, "  Current         %s\n", si(s->src_current, "A", b1, sizeof b1));
    fprintf(out, "  Power supplied  %s\n", si(c->src_volts * s->src_current, "W", b1, sizeof b1));
    fprintf(out, "  Power used      %-14s   (all resistors)\n", si(s->total_power, "W", b1, sizeof b1));
}

static void save_results(const Circuit *c, const Solution *s)
{
    FILE *fp = fopen(RESULTS_FILE, "w");

    if (fp == NULL) {
        printf("\n  ! Could not create %s.\n", RESULTS_FILE);
        return;
    }
    fprintf(fp, "MiniSPICE results\n");
    fprintf(fp, "Nodes: %d (ground = node 0), resistors: %d\n", c->nodes, c->count);
    print_results(fp, c, s);
    fclose(fp);
    printf("\n  Saved to %s.\n", RESULTS_FILE);
}

/* ------------------------------------------------------------------ */
/*  Main                                                              */
/* ------------------------------------------------------------------ */

int main(void)
{
    Circuit  c = { 0 };
    Solution s = { 0 };
    char prompt[LINE_LEN];

    banner();
    snprintf(prompt, sizeof prompt, "  Number of nodes, ground included [2-%d] : ", MAX_NODES);
    c.nodes = read_int(prompt, 2, MAX_NODES);

    for (;;) {
        print_menu(&c, &s);

        switch (read_int("  > ", 1, 6)) {
        case 1:
            input_resistors(&c);
            s.ready = 0;                 /* circuit changed */
            break;
        case 2:
            input_source(&c);
            s.ready = 0;
            break;
        case 3:
            solve(&c, &s);
            break;
        case 4:
            if (s.ready)
                print_results(stdout, &c, &s);
            else
                printf("\n  ! Nothing to show yet. Solve the circuit first (option 3).\n");
            break;
        case 5:
            if (s.ready)
                save_results(&c, &s);
            else
                printf("\n  ! Nothing to save yet. Solve the circuit first (option 3).\n");
            break;
        case 6:
            printf("\n  Goodbye!\n\n");
            return 0;
        }
    }
}
