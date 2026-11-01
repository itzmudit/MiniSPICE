/*
 * ============================================================
 *  MiniSPICE: DC Circuit Solver in C
 *  Software Development Fundamentals Lab-  1 (24B15CS111)
 *  JIIT Noida, B.Tech ECE 1st Year, Batch A1
 *  Supervisor : Dr. Taj Alam
 *  Team       : Mudit Jindal (2601020021), Riyansh Pandey (2601020026),
 *               Aniket Rastogi (2601020031), Prakhar Mishra (2601020007)
 *
 *  Solves DC circuits made of resistors and one voltage source using
 *  nodal analysis (KCL) and Gaussian elimination with partial pivoting.
 *
 *  Assumptions:
 *    - Node 0 is ground.
 *    - Negative terminal of the voltage source is connected to node 0.
 *
 *  Compile : gcc minispice.c -o minispice -lm
 *  Run     : ./minispice        (Windows: minispice.exe)
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_NODES 10      /* non-ground nodes: 1 to 10 */
#define MAX_COMP  20      /* max components in a circuit */
#define EPS       1e-12   /* pivot smaller than this = singular matrix */

/* One circuit element: resistor ('R') or voltage source ('V') */
typedef struct {
    char   type;    /* 'R' or 'V' */
    int    n1;      /* first node  (+ terminal for the source) */
    int    n2;      /* second node (- terminal for the source) */
    double value;   /* resistance in ohm, or voltage in volt */
} Component;

/* ---------------- function prototypes ---------------- */
void   clearInput(void);
int    readInt(const char *prompt);
double readDouble(const char *prompt);

void   inputComponents(Component *comp, int *count, int numNodes, char type);
int    findSource(const Component *comp, int count);
void   showCircuit(const Component *comp, int count, int numNodes);
int    checkCircuit(const Component *comp, int count, int numNodes);

void   buildMatrix(const Component *comp, int count, int n,
                   double G[][MAX_NODES], double *I);
int    gaussianElimination(int n, double G[][MAX_NODES], double *I, double *V);
double nodeVoltage(const double *V, int node);
void   printResults(FILE *fp, const Component *comp, int count,
                    int n, const double *V);

/* ======================= main ======================= */
int main(void)
{
    Component comp[MAX_COMP];
    double G[MAX_NODES][MAX_NODES]; 
    double I[MAX_NODES];
    double V[MAX_NODES];
    int count = 0;       /* number of components entered */
    int numNodes = 0;    /* number of non-ground nodes   */
    int choice;

    printf("=============================================\n");
    printf("        MiniSPICE : DC Circuit Solver\n");
    printf("=============================================\n");
    printf("Node 0 is ground. Source (-) terminal must be at node 0.\n");

    do {
        printf("\n---------------- MENU ----------------\n");
        printf(" 1. Set number of nodes (clears circuit)\n");
        printf(" 2. Add resistor\n");
        printf(" 3. Add / replace voltage source\n");
        printf(" 4. Show circuit\n");
        printf(" 5. Solve circuit\n");
        printf(" 0. Exit\n");
        printf("--------------------------------------\n");
        choice = readInt("Enter choice: ");

        switch (choice) {
        case 1:
            do {
                numNodes = readInt("Number of nodes excluding ground (1-10): ");
                if (numNodes < 1 || numNodes > MAX_NODES)
                    printf("  Error: must be between 1 and %d.\n", MAX_NODES);
            } while (numNodes < 1 || numNodes > MAX_NODES);
            count = 0;
            printf("  Circuit cleared. Nodes available: 0 (ground) to %d.\n",
                   numNodes);
            break;

        case 2:
            if (numNodes == 0) { printf("  Set the number of nodes first (option 1).\n"); break; }
            inputComponents(comp, &count, numNodes, 'R');
            break;

        case 3:
            if (numNodes == 0) { printf("  Set the number of nodes first (option 1).\n"); break; }
            inputComponents(comp, &count, numNodes, 'V');
            break;

        case 4:
            showCircuit(comp, count, numNodes);
            break;

        case 5:
            if (!checkCircuit(comp, count, numNodes))
                break;
            buildMatrix(comp, count, numNodes, G, I);
            if (!gaussianElimination(numNodes, G, I, V)) {
                printf("\n  Error: matrix is singular, circuit cannot be solved.\n");
                printf("  Some part of the circuit is floating (not connected to\n");
                printf("  ground or the source). Check the connections.\n");
                break;
            }
            printResults(stdout, comp, count, numNodes, V);

            if (readInt("\nSave results to results.txt? (1 = yes, 0 = no): ") == 1) {
                FILE *fp = fopen("results.txt", "w");
                if (fp == NULL) {
                    printf("  Error: could not open results.txt for writing.\n");
                } else {
                    printResults(fp, comp, count, numNodes, V);
                    fclose(fp);
                    printf("  Results saved to results.txt\n");
                }
            }
            break;

        case 0:
            printf("Exiting MiniSPICE. Goodbye.\n");
            break;

        default:
            printf("  Invalid choice, try again.\n");
        }
    } while (choice != 0);

    return 0;
}

/* ================ input helper functions ================ */

/* Throw away the rest of the current input line */
void clearInput(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

/* Keep asking until the user enters a valid integer */
int readInt(const char *prompt)
{
    int x;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &x) == 1) {
            clearInput();
            return x;
        }
        if (feof(stdin)) {
            printf("\nInput ended.\n");
            exit(0);
        }
        printf("  Invalid input, enter a whole number.\n");
        clearInput();
    }
}

/* Keep asking until the user enters a valid number */
double readDouble(const char *prompt)
{
    double x;
    while (1) {
        printf("%s", prompt);
        if (scanf("%lf", &x) == 1) {
            clearInput();
            return x;
        }
        if (feof(stdin)) {
            printf("\nInput ended.\n");
            exit(0);
        }
        printf("  Invalid input, enter a number.\n");
        clearInput();
    }
}

/* ================ circuit entry functions ================ */

/* Returns index of the voltage source in comp[], or -1 if none */
int findSource(const Component *comp, int count)
{
    int i;
    for (i = 0; i < count; i++)
        if (comp[i].type == 'V')
            return i;
    return -1;
}

/* Reads one resistor or the voltage source with input checks */
void inputComponents(Component *comp, int *count, int numNodes, char type)
{
    Component c;
    int src = findSource(comp, *count);

    if (type == 'R' && *count >= MAX_COMP) {
        printf("  Error: maximum %d components reached.\n", MAX_COMP);
        return;
    }
    if (type == 'V' && src == -1 && *count >= MAX_COMP) {
        printf("  Error: maximum %d components reached.\n", MAX_COMP);
        return;
    }

    c.type = type;

    if (type == 'R') {
        printf("\n  Enter resistor details (nodes 0 to %d)\n", numNodes);
        while (1) {
            c.n1 = readInt("  Node A: ");
            c.n2 = readInt("  Node B: ");
            if (c.n1 < 0 || c.n1 > numNodes || c.n2 < 0 || c.n2 > numNodes)
                printf("  Error: node numbers must be from 0 to %d.\n", numNodes);
            else if (c.n1 == c.n2)
                printf("  Error: both ends cannot be on the same node.\n");
            else
                break;
        }
        while (1) {
            c.value = readDouble("  Resistance (ohm): ");
            if (c.value > 0)
                break;
            printf("  Error: resistance must be positive.\n");
        }
        comp[*count] = c;
        (*count)++;
        printf("  Resistor R%d added.\n", *count);
    } else {
        printf("\n  Enter voltage source details\n");
        while (1) {
            c.n1 = readInt("  Positive (+) node: ");
            c.n2 = readInt("  Negative (-) node (must be 0): ");
            if (c.n1 < 1 || c.n1 > numNodes)
                printf("  Error: + node must be from 1 to %d.\n", numNodes);
            else if (c.n2 != 0)
                printf("  Error: - terminal must be connected to ground (node 0).\n");
            else
                break;
        }
        c.value = readDouble("  Voltage (V): ");

        if (src != -1) {
            comp[src] = c;                 /* only one source allowed */
            printf("  Old source replaced (only one source is allowed).\n");
        } else {
            comp[*count] = c;
            (*count)++;
            printf("  Voltage source added.\n");
        }
    }
}

/* Prints the list of components */
void showCircuit(const Component *comp, int count, int numNodes)
{
    int i, r = 0;
    if (count == 0) {
        printf("  Circuit is empty.\n");
        return;
    }
    printf("\n  Nodes: 0 (ground) to %d   Components: %d\n", numNodes, count);
    printf("  %-10s %-6s %-6s %s\n", "Element", "Node1", "Node2", "Value");
    for (i = 0; i < count; i++) {
        if (comp[i].type == 'R') {
            r++;
            printf("  R%-9d %-6d %-6d %.4f ohm\n",
                   r, comp[i].n1, comp[i].n2, comp[i].value);
        } else {
            printf("  %-10s %-6d %-6d %.4f V\n",
                   "Vsource", comp[i].n1, comp[i].n2, comp[i].value);
        }
    }
}

/* Checks the circuit before solving. Returns 1 if OK, 0 if not */
int checkCircuit(const Component *comp, int count, int numNodes)
{
    int links[MAX_NODES + 1] = {0};
    int i, ok = 1, resistors = 0;

    if (numNodes == 0 || count == 0) {
        printf("  Error: circuit is empty.\n");
        return 0;
    }
    if (findSource(comp, count) == -1) {
        printf("  Error: no voltage source in the circuit.\n");
        return 0;
    }
    for (i = 0; i < count; i++) {
        links[comp[i].n1]++;
        links[comp[i].n2]++;
        if (comp[i].type == 'R')
            resistors++;
    }
    if (resistors == 0) {
        printf("  Error: add at least one resistor.\n");
        return 0;
    }
    for (i = 1; i <= numNodes; i++) {
        if (links[i] == 0) {
            printf("  Error: node %d is floating (nothing is connected to it).\n", i);
            ok = 0;
        }
    }
    return ok;
}

/* ================ solver functions ================ */

/* Forms G and I from the components using KCL.
   Node k is stored at index k-1 (ground is left out). */
void buildMatrix(const Component *comp, int count, int n,
                 double G[][MAX_NODES], double *I)
{
    int i, j, a, b, k;
    double g;

    for (i = 0; i < n; i++) {
        I[i] = 0.0;
        for (j = 0; j < n; j++)
            G[i][j] = 0.0;
    }

    /* stamp every resistor */
    for (i = 0; i < count; i++) {
        if (comp[i].type != 'R')
            continue;
        g = 1.0 / comp[i].value;
        a = comp[i].n1 - 1;
        b = comp[i].n2 - 1;
        if (a >= 0) G[a][a] += g;
        if (b >= 0) G[b][b] += g;
        if (a >= 0 && b >= 0) {
            G[a][b] -= g;
            G[b][a] -= g;
        }
    }

    /* fix the row of the source node: V(k) = source voltage */
    i = findSource(comp, count);
    k = comp[i].n1 - 1;
    for (j = 0; j < n; j++)
        G[k][j] = 0.0;
    G[k][k] = 1.0;
    I[k] = comp[i].value;
}

/* Solves G x V = I by Gaussian elimination with partial pivoting.
   Returns 1 on success, 0 if the matrix is singular. */
int gaussianElimination(int n, double G[][MAX_NODES], double *I, double *V)
{
    int i, j, k, p;
    double maxVal, factor, temp, sum;

    for (k = 0; k < n; k++) {
        /* find the row with the largest value in column k */
        p = k;
        maxVal = fabs(G[k][k]);
        for (i = k + 1; i < n; i++) {
            if (fabs(G[i][k]) > maxVal) {
                maxVal = fabs(G[i][k]);
                p = i;
            }
        }
        if (maxVal < EPS)
            return 0;                       /* singular matrix */

        /* swap row p and row k */
        if (p != k) {
            for (j = 0; j < n; j++) {
                temp = G[k][j];
                G[k][j] = G[p][j];
                G[p][j] = temp;
            }
            temp = I[k];
            I[k] = I[p];
            I[p] = temp;
        }

        /* make zeros below the pivot */
        for (i = k + 1; i < n; i++) {
            factor = G[i][k] / G[k][k];
            for (j = k; j < n; j++)
                G[i][j] -= factor * G[k][j];
            I[i] -= factor * I[k];
        }
    }

    /* back substitution */
    for (i = n - 1; i >= 0; i--) {
        sum = I[i];
        for (j = i + 1; j < n; j++)
            sum -= G[i][j] * V[j];
        V[i] = sum / G[i][i];
    }
    return 1;
}

/* Voltage of any node, ground included */
double nodeVoltage(const double *V, int node)
{
    return (node == 0) ? 0.0 : V[node - 1];
}

/* Prints results to the screen (stdout) or to a file */
void printResults(FILE *fp, const Component *comp, int count,
                  int n, const double *V)
{
    int i, r = 0, s, k;
    double cur, pw, totalR = 0.0, srcCur = 0.0, srcPow;

    fprintf(fp, "\n=============== MiniSPICE RESULTS ===============\n");
    fprintf(fp, "\nNode voltages (with respect to ground):\n");
    fprintf(fp, "  V(0) = %10.4f V  (ground)\n", 0.0);
    for (i = 1; i <= n; i++)
        fprintf(fp, "  V(%d) = %10.4f V\n", i, nodeVoltage(V, i));

    fprintf(fp, "\nResistors (current flows from Node A to Node B if positive):\n");
    fprintf(fp, "  %-5s %-5s %-5s %12s %12s %12s\n",
            "Name", "A", "B", "R (ohm)", "I (A)", "P (W)");

    s = findSource(comp, count);
    k = comp[s].n1;                        /* source + node */

    for (i = 0; i < count; i++) {
        if (comp[i].type != 'R')
            continue;
        r++;
        cur = (nodeVoltage(V, comp[i].n1) - nodeVoltage(V, comp[i].n2))
              / comp[i].value;
        pw = cur * cur * comp[i].value;   /* P = I^2 R */
        totalR += pw;
        fprintf(fp, "  R%-4d %-5d %-5d %12.4f %12.4f %12.4f\n",
                r, comp[i].n1, comp[i].n2, comp[i].value, cur, pw);

        /* current leaving the source node through this resistor */
        if (comp[i].n1 == k) srcCur += cur;
        if (comp[i].n2 == k) srcCur -= cur;
    }

    srcPow = comp[s].value * srcCur;
    fprintf(fp, "\nVoltage source (+ at node %d, - at node 0):\n", k);
    fprintf(fp, "  Voltage        = %10.4f V\n", comp[s].value);
    fprintf(fp, "  Current given  = %10.4f A\n", srcCur);
    fprintf(fp, "  Power given    = %10.4f W\n", srcPow);

    fprintf(fp, "\nPower check:\n");
    fprintf(fp, "  Total power used by resistors = %10.4f W\n", totalR);
    fprintf(fp, "  Power given by source         = %10.4f W\n", srcPow);
    fprintf(fp, "  %s\n", fabs(totalR - srcPow) < 1e-6 ? "Balanced (OK)"
                                                         : "Not balanced!");
    fprintf(fp, "=================================================\n");
}
