/*
 * ============================================================
 *  MiniSPICE: DC Circuit Solver in C
 *  Software Development Fundamentals Lab-  1 (24B15CS111)
 *  JIIT Noida, B.Tech ECE 1st Year, Batch A1
 *  Supervisor : Dr. Taj Alam
 *  Team       : Mudit Jindal (2601020021), Riyansh Pandey (2601020026),
 *               Aniket Rastogi (2601020031), Prakhar Mishra (2601020007)
 *
 *  Solves DC circuits made of resistors and one voltage source using nodal analysis (KCL) and Gaussian elimination.
 * ============================================================
 */

// Header files: stdio for input/output, math for fabs(), stdlib for malloc/calloc/free
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

// Limits of the program: at most 10 nodes and 20 resistors
#define MAX_NODES 10
#define MAX_COMP 20

// Removes leftover characters (like Enter or wrong letters) from the input buffer after scanf
void clearBuffer()
{
    while (getchar() != '\n');
}

// Structure for one resistor: the two nodes it connects and its resistance in ohms
typedef struct
{
    int n1;
    int n2;
    double value;
} Resistor;

// List of every resistor entered by the user, and how many are stored
Resistor Rlist[MAX_COMP];
int count = 0; 

// Builds the conductance matrix G and the right-hand side I using KCL at every node (ground row/column left out)
void buildMatrix(double **resist, int nodes, int posNode, double volt, double Left_Grid[][MAX_NODES], double Right_Side_List[])
{
    int i, j;

    // Step 1: set the whole matrix and RHS to zero
    for (i = 1; i < nodes; i++)
    {
        Right_Side_List[i] = 0.0;
        for (j = 1; j < nodes; j++)
        {
            Left_Grid[i][j] = 0.0;
        }
    }

    // Step 2: fill one equation (row) for every non-ground node
    for (i = 1; i < nodes; i++)
    {
        // Battery node: its voltage is known, so the row becomes V[posNode] = volt
        if (i == posNode)
        {
            Left_Grid[i][i] = 1.0;
            Right_Side_List[i] = volt;
        }
        // Other nodes: add conductance g = 1/R of every resistor connected to this node
        else
        {
            for (j = 0; j < nodes; j++)
            {
                if (i != j && resist[i][j] > 0.0)
                {
                    double g_val = 1.0 / resist[i][j];
                    Left_Grid[i][i] = Left_Grid[i][i] + g_val;

                    if (j != 0)
                    {
                        Left_Grid[i][j] = Left_Grid[i][j] - g_val;
                    }
                }
            }
        }
    }
}

// Solves G x V = I using Gaussian elimination with partial pivoting. Returns 1 if solved, 0 if the matrix is singular (floating node)
int solveGaussian(double G_LHS[][MAX_NODES], double V[], double RHS[], int NoDe)
{
    int a, b, c;
    int maxRow;
    double temp;

    for (a = 1; a < NoDe; a++)
    {
        // Partial pivoting: find the row with the largest value in column a
        maxRow = a;
        for (b = a + 1; b < NoDe; b++)
        {
            if (fabs(G_LHS[b][a]) > fabs(G_LHS[maxRow][a]))
            {
                maxRow = b;
            }
        }

        // Pivot is almost zero, so the circuit cannot be solved
        if (fabs(G_LHS[maxRow][a]) < 1e-12)
        {
            return 0;
        }   

        // Swap the current row with the pivot row (in both G and RHS)
        if (maxRow != a)
        {
            for (c = 1; c < NoDe; c++)
            {
                temp = G_LHS[a][c];
                G_LHS[a][c] = G_LHS[maxRow][c];
                G_LHS[maxRow][c] = temp;
            }
            temp = RHS[a];
            RHS[a] = RHS[maxRow];
            RHS[maxRow] = temp;
        }

        // Forward elimination: make every value below the pivot zero
        for (b = a + 1; b < NoDe; b++)
        {
            double factor = G_LHS[b][a] / G_LHS[a][a];
            for (c = a; c < NoDe; c++)
            {
                G_LHS[b][c] = G_LHS[b][c] - factor * G_LHS[a][c];
            }
            RHS[b] = RHS[b] - factor * RHS[a];
        }
    }

    // Back substitution: find node voltages from the last row upwards
    for (b = NoDe - 1; b >= 1; b--)
    {
        V[b] = RHS[b];
        for (c = b + 1; c < NoDe; c++)
        {
            V[b] = V[b] - (G_LHS[b][c] * V[c]);
        }
        V[b] = V[b] / G_LHS[b][b];
    }

    V[0] = 0.0; 
    return 1;
}

// Main program: input, menu, solving and output
int main()
{
    double **resist, volt = 0;
    int posNode = 0;
    char n[50];
    int nodes = 0;
    int valid = 0;
    
    // Title screen
    printf("\n+----------------------------------------------------------+\n");
    printf("|                       MiniSPICE                          |\n");
    printf("|              DC Circuit Solver | Nodal Analysis          |\n");
    printf("+----------------------------------------------------------+\n\n");
    printf("GROUND REFERENCE : Node 0\n");
    printf("METHOD           : Nodal analysis + Gaussian elimination\n");
    printf("SUPPORTED        : Resistors and one DC voltage source\n\n");
    printf("Minimum Nodes: 2\nMaximum Nodes: 10\n");
    // Read the number of nodes as a string and accept only 2 to 10
    do
    {
        printf("Number of nodes (including ground): ");
        scanf("%49s", n);

        if (n[1] == '\0' && n[0] >= '2' && n[0] <= '9')
        {
            nodes = n[0] - '0';
            valid = 1;
        }
        else if (n[0] == '1' && n[1] == '0' && n[2] == '\0')
        {
            nodes = 10;
            valid = 1;
        }
        else
        {
            printf("[INPUT ERROR] Please enter a whole number from 2 to 10.\n");
        }
    } while (valid == 0);

    // Dynamic memory for the resistance matrix resist[nodes][nodes]; calloc sets all values to 0 (0 = no resistor)
    resist = (double **)malloc(nodes * sizeof(double *));

    if (resist == NULL)
    {
        printf("[ERROR] Could not allocate circuit memory.\n");
        return 1;
    }

    for (int i = 0; i < nodes; i++)
    {
        resist[i] = (double *)calloc(nodes, sizeof(double));
        if (resist[i] == NULL)
        {
            printf("[ERROR] Could not allocate node memory.\n");
            for (int l = 0; l < i; l++)
            {
                free(resist[l]);
            }
            free(resist);
            return 1;
        }
    }

    int i, j;
    int choice = 0;
    char input[50];

    // Results of the last solve, kept outside the menu so that options 4 and 5 can use them
    double V[MAX_NODES] = {0.0};
    double RCurrent[MAX_COMP];
    double RPower[MAX_COMP];
    double BCurrent = 0;  // current given by the battery
    double TPower = 0;    // total power of all resistors
    int solved = 0;  // 1 = results ready, 0 = solve first

    // Main menu loop: runs until the user chooses Exit (6)
    do
    {
        printf("\n+------------------------ MAIN MENU -----------------------+\n");
        printf("Select an operation:\n");
        printf("  1.  Set resistor values\n  2.  Set voltage source\n  3.  Solve circuit\n  4.  View results\n  5.  Save results to results.txt\n  6.  Exit\n");
        printf("Choose an option [1-6]: ");
        scanf("%49s", input);

        if (input[1] == '\0' && input[0] >= '1' && input[0] <= '6')
        {
            choice = input[0] - '0';
            switch (choice)
            {
            // Option 1: enter resistors for every node pair (parallel resistors allowed)
            case 1:
            {
                count = 0;
                solved = 0;  // circuit changed, so old results are no longer valid
                printf("\n+-------------------- RESISTOR SETUP ----------------------+\n");
                printf("Enter resistance in ohms for each node pair.\n");
                printf("Enter 0 when no resistor is connected; parallel resistors are supported.\n\n");
                // Ask every node pair once: (0,1), (0,2), ... (1,2), ...
                for (i = 0; i < nodes; i++)
                {
                    for (j = i + 1; j < nodes; j++)
                    {
                        int more = 0;
                        double r = 0;
                        double total_g = 0; 

                        do
                        {
                            // Read the resistance and reject letters and negative values
                            do
                            {
                                printf("Resistance between Node %d and Node %d (ohms): ", i, j);
                                if (scanf("%lf", &r) != 1)
                                {
                                    clearBuffer();
                                    printf("[INPUT ERROR] Please enter a numeric resistance.\n");
                                    r = -1;
                                    continue;
                                }
                                clearBuffer();

                                if (r < 0)
                                {
                                    printf("[INPUT ERROR] Resistance cannot be negative. Try again.\n");
                                }
                            } while (r < 0);

                            more = 0;
                            if (r > 0)
                            {
                                // Store the resistor in Rlist and add its conductance (parallel: 1/R values add up)
                                if (count < MAX_COMP)
                                {
                                    total_g = total_g + 1.0 / r; 
                                    Rlist[count].n1 = i;
                                    Rlist[count].n2 = j;
                                    Rlist[count].value = r;
                                    count++;
                                }
                                else
                                {
                                    printf("[LIMIT] Maximum of %d resistors reached.\n", MAX_COMP);
                                }

                                // Ask whether to add another parallel resistor on the same pair
                                printf("  1.  Add another parallel resistor between Nodes %d and %d\n", i, j);
                                if (i == nodes - 2 && j == nodes - 1)
                                {
                                    printf("  2.  Finish resistor setup\n");
                                }
                                else
                                {
                                    printf("  2.  Continue to the next node pair\n");
                                }
                                printf("Choose [1 or 2]: ");
                                while ( (scanf("%d", &more) != 1) ||(( (more) != 1) && (more) != 2))
                                {
                                    clearBuffer();
                                    printf("[INPUT ERROR] Choose 1 or 2: ");
                                }
                                clearBuffer();

                                switch (more)
                                {
                                case 1:
                                    break;
                                case 2:
                                    more = 0;
                                    break;
                                }
                            }
                        } while (more == 1);

                        // Store the equivalent resistance of this pair in the matrix (same value both ways)
                        if (total_g > 0)
                        {
                            resist[i][j] = 1.0 / total_g;
                        }
                        else
                        {
                            resist[i][j] = 0;
                        }
                        resist[j][i] = resist[i][j];
                    }
                }
                break;
            }
            // Option 2: battery setup (negative terminal is always at ground, Node 0)
            case 2:
            {
                solved = 0;  // circuit changed, so old results are no longer valid
                printf("\n+--------------------- BATTERY SETUP ----------------------+\n");
                printf("The negative (-) terminal is grounded at Node 0.\n");
                printf("Positive terminal node [1-%d]: ", nodes - 1);
                posNode = 0;
                while (scanf("%d", &posNode) != 1)
                {
                    clearBuffer();
                    printf("[INPUT ERROR] Enter a node number [1-%d]: ", nodes - 1);
                }
                clearBuffer();
                while (posNode < 1 || posNode > nodes - 1)
                {
                    printf("[INPUT ERROR] Node must be [1-%d]. Try again: ", nodes - 1);
                    while (scanf("%d", &posNode) != 1)
                    {
                        clearBuffer();
                        printf("[INPUT ERROR] Enter a node number [1-%d]: ", nodes - 1);
                    }
                    clearBuffer();
                }
                // Read the battery voltage
                printf("Source voltage (volts): ");
                while (scanf("%lf", &volt) != 1)
                {
                    clearBuffer();
                    printf("[INPUT ERROR] Enter a numeric value: ");
                }
                clearBuffer();
                break;
            }
            // Option 3: build the matrix, solve it and calculate currents and power
            case 3:
            {
                if (posNode == 0)
                {
                    printf("[SETUP INCOMPLETE] Add the voltage source before solving.\n");
                    break;
                }

                double Left_Grid[MAX_NODES][MAX_NODES];
                double Right_Side_List[MAX_NODES];

                printf("\n+--------------------- CIRCUIT ANALYSIS -------------------+\n");
                printf("Building the nodal conductance matrix...\n\n");

                // Build G and I, then print them
                buildMatrix(resist, nodes, posNode, volt, Left_Grid, Right_Side_List);

                printf("NODAL CONDUCTANCE MATRIX  (COEFFICIENTS | RHS)\n");
                printf("Rows and columns are nodes 1 through %d.\n\n", nodes - 1);
                for (i = 1; i < nodes; i++)
                {
                    for (j = 1; j < nodes; j++)
                    {
                        printf("%8.4f\t", Left_Grid[i][j]);
                    }
                    printf(" |  %8.4f\n", Right_Side_List[i]);
                }

                // Clear old results before solving again
                for (i = 0; i < MAX_NODES; i++)
                {
                    V[i] = 0.0;
                }
                solved = 0;

                // Solve the equations; show an error if a node is floating
                if (solveGaussian(Left_Grid, V, Right_Side_List, nodes) == 0)
                {
                    printf("\n[SOLVE ERROR] Circuit matrix is singular.\n");
                    printf("Check that every node has a conductive path to ground.\n");
                    break;
                }

                // Current (Ohm's law) and power (I*I*R) of every resistor, total power and battery current
                TPower = 0;
                BCurrent = 0;
                for (int k = 0; k < count; k++)
                {
                    int a = Rlist[k].n1;
                    int b = Rlist[k].n2;

                    RCurrent[k] = (V[a] - V[b]) / Rlist[k].value;
                    RPower[k] = RCurrent[k] * RCurrent[k] * Rlist[k].value;
                    TPower += RPower[k];

                    // Battery current = total current leaving the battery node (KCL)
                    if (a == posNode)
                    {
                        BCurrent += RCurrent[k];
                    }
                    else if (b == posNode)
                    {
                        BCurrent -= RCurrent[k];
                    }
                }

                solved = 1;
                printf("\n[SUCCESS] Circuit solved. Choose option 4 to view the results.\n");
                break;
            }
            // Option 4: show the results on the screen
            case 4:
            {
                if (solved == 0)
                {
                    printf("[NOT SOLVED] Solve the circuit first (option 3).\n");
                    break;
                }

                printf("\n+---------------------- SOLUTION --------------------------+\n");
                printf("\nNODE VOLTAGES\n");
                printf("------------------------------------------------------------\n");
                printf("Node 0  |  0.0000 V\n");
                for (i = 1; i < nodes; i++)
                {
                    printf("Node %-2d |  %10.4f V\n", i, V[i]);
                }

                printf("\n+------------------- CURRENT & POWER ----------------------+\n");
                printf("\nRESISTOR CURRENT & POWER\n");
                printf("------------------------------------------------------------\n");
                printf("ID   CONNECTION       RESISTANCE       CURRENT        POWER\n");
                printf("------------------------------------------------------------\n");
                for (int k = 0; k < count; k++)
                {
                    printf("R%-3d N%d-N%d          %8.4f ohm   %8.4f A   %8.4f W\n", k + 1, Rlist[k].n1, Rlist[k].n2, Rlist[k].value, RCurrent[k], RPower[k]);
                }

                printf("\nSOURCE SUMMARY\n");
                printf("------------------------------------------------------------\n");
                printf("Source voltage             : %10.4f V\n", volt);
                printf("Source current             : %10.4f A\n", BCurrent);
                printf("Source power               : %10.4f W\n", volt * BCurrent);
                printf("Total resistor dissipation : %9.4f W\n", TPower);
                break;
            }
            // Option 5: write the results into results.txt
            case 5:
            {
                if (solved == 0)
                {
                    printf("[NOT SOLVED] Solve the circuit first (option 3).\n");
                    break;
                }

                FILE *fp = fopen("results.txt", "w");

                if (fp != NULL)
                {
                    fprintf(fp, "+============================================================+\n");
                    fprintf(fp, "|                    MiniSPICE RESULTS                       |\n");
                    fprintf(fp, "|           DC Circuit Analysis  |  Nodal Method             |\n");
                    fprintf(fp, "+============================================================+\n\n");
                    fprintf(fp, "CIRCUIT SETUP\n");
                    fprintf(fp, "--------------------------------------------------------------\n");
                    fprintf(fp, "Total nodes (including ground) : %d\n", nodes);
                    fprintf(fp, "Ground reference               : Node 0\n");
                    fprintf(fp, "Source positive terminal       : Node %d\n", posNode);
                    fprintf(fp, "Source voltage                 : %.4f V\n\n", volt);

                    fprintf(fp, "NODE VOLTAGES\n");
                    fprintf(fp, "--------------------------------------------------------------\n");
                    fprintf(fp, "Node      Voltage        Reference\n");
                    fprintf(fp, "--------------------------------------------------------------\n");
                    fprintf(fp, "0         %10.4f V    Ground\n", V[0]);
                    for (i = 1; i < nodes; i++)
                    {
                        fprintf(fp, "%-9d %10.4f V\n", i, V[i]);
                    }

                    fprintf(fp, "\nRESISTOR CURRENT & POWER\n");
                    fprintf(fp, "--------------------------------------------------------------\n");
                    fprintf(fp, "ID   Nodes     Resistance       Current        Power\n");
                    fprintf(fp, "--------------------------------------------------------------\n");
                    for (int k = 0; k < count; k++)
                    {
                        fprintf(fp, "R%-3d N%-2d-N%-2d  %9.4f ohm   %9.4f A   %9.4f W\n", k + 1, Rlist[k].n1, Rlist[k].n2, Rlist[k].value, RCurrent[k], RPower[k]);
                    }

                    fprintf(fp, "\nSOURCE & POWER SUMMARY\n");
                    fprintf(fp, "--------------------------------------------------------------\n");
                    fprintf(fp, "Source voltage                  : %10.4f V\n", volt);
                    fprintf(fp, "Source current                  : %10.4f A\n", BCurrent);
                    fprintf(fp, "Source power                    : %10.4f W\n", volt * BCurrent);
                    fprintf(fp, "Total resistor power dissipated : %10.4f W\n", TPower);
                    fprintf(fp, "\nNote: Current direction follows the listed node order.\n");
                    fprintf(fp, "      Positive source current is delivered from the source.\n");

                    fclose(fp);
                    printf("\n[SUCCESS] Results successfully saved to 'results.txt'!\n");
                }
                else
                {
                    printf("\n[ERROR] Could not create results.txt!\n");
                }
                break;
            }
            // Option 6: exit the program
            case 6:
            {
                printf("\nThank you for using MiniSPICE.\n          Goodbye!\n");
                break;
            }
            }
        }
        else
        {
            printf("[INPUT ERROR] Choose a menu option from 1 to 6.\n");
            choice = 0;
        }
    } while (choice != 6);

    // Free the dynamic memory before the program ends
    for (int l = 0; l < nodes; l++)
    {
        free(resist[l]);
    }
    free(resist);

    return 0;
}
