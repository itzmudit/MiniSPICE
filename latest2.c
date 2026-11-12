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
 *  nodal analysis (KCL) and Gaussian elimination.
 * ============================================================
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define MAX_NODES 10
#define MAX_COMP 20

void clearBuffer()
{
    while (getchar() != '\n');
}

typedef struct
{
    int n1;
    int n2;
    double value;
} Resistor;

Resistor Rlist[MAX_COMP];
int count = 0; 

void buildMatrix(double **resist, int nodes, int posNode, double volt, double Left_Grid[][MAX_NODES], double Right_Side_List[])
{
    int i, j;

    for (i = 1; i < nodes; i++)
    {
        Right_Side_List[i] = 0.0;
        for (j = 1; j < nodes; j++)
        {
            Left_Grid[i][j] = 0.0;
        }
    }

    for (i = 1; i < nodes; i++)
    {
        if (i == posNode)
        {
            Left_Grid[i][i] = 1.0;
            Right_Side_List[i] = volt;
        }
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

int solveGaussian(double G_LHS[][MAX_NODES], double V[], double RHS[], int NoDe)
{
    int a, b, c;
    int maxRow;
    double temp;

    for (a = 1; a < NoDe; a++)
    {
        maxRow = a;
        for (b = a + 1; b < NoDe; b++)
        {
            if (fabs(G_LHS[b][a]) > fabs(G_LHS[maxRow][a]))
            {
                maxRow = b;
            }
        }

        if (fabs(G_LHS[maxRow][a]) < 1e-12)
        {
            return 0;
        }   

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

int main()
{
    double **resist, volt = 0;
    int posNode = 0;
    char n[50];
    int nodes = 0;
    int valid = 0;
    
    printf("Assumption: Node 0 is Ground Node.\n");

    do
    {
        printf("Enter the total number of nodes (Including ground node): ");
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
            printf("Enter valid number of nodes!\nMinimum nodes: 2\nMaximum nodes: 10\n");
        }
    } while (valid == 0);

    resist = (double **)malloc(nodes * sizeof(double *));

    if (resist == NULL)
    {
        printf("Memory allocation Failed.\n");
        return 1;
    }

    for (int i = 0; i < nodes; i++)
    {
        resist[i] = (double *)calloc(nodes, sizeof(double));
        if (resist[i] == NULL)
        {
            printf("Memory allocation failed.\n");
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

    do
    {
        printf("\nSelect:  \n");
        printf("1. Resistance \n2. Battery\n3. Solve Circuit\n4. Exit\n");
        printf("Enter choice: ");
        scanf("%49s", input);

        if (input[1] == '\0' && input[0] >= '1' && input[0] <= '4')
        {
            choice = input[0] - '0';
            switch (choice)
            {
            case 1:
            {
                count = 0;
                printf("Enter 0 if no resistor is connected.\n");
                for (i = 0; i < nodes; i++)
                {
                    for (j = i + 1; j < nodes; j++)
                    {
                        int more = 0;
                        double r = 0;
                        double total_g = 0; 

                        do
                        {
                            do
                            {
                                printf("Enter the value of resistor between Node %d and Node %d : ", i, j);
                                if (scanf("%lf", &r) != 1)
                                {
                                    clearBuffer();
                                    printf("Invalid input! Enter a number.\n");
                                    r = -1;
                                    continue;
                                }
                                clearBuffer();

                                if (r < 0)
                                {
                                    printf("Resistance cannot be negative! Enter again.\n");
                                }
                            } while (r < 0);

                            more = 0;
                            if (r > 0)
                            {
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
                                    printf("Maximum %d resistors allowed!\n", MAX_COMP);
                                }

                                printf("1. Add another resistor between %d and %d.\n", i, j);
                                if (i == nodes - 2 && j == nodes - 1)
                                {
                                    printf("2. Done (all pairs entered).\n");
                                }
                                else
                                {
                                    printf("2. Add next pair.\n");
                                }
                                printf("Enter choice: ");
                                while (scanf("%d", &more) != 1)
                                {
                                    clearBuffer();
                                    printf("Invalid input! Enter 1 or 2: ");
                                }
                                clearBuffer();

                                switch (more)
                                {
                                case 1:
                                    break;
                                case 2:
                                    more = 0;
                                    break;
                                default:
                                    printf("Invalid choice, moving to next pair.\n");
                                    more = 0;
                                }
                            }
                        } while (more == 1);

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
            case 2:
            {
                printf("Negative (-) terminal of battery is connected to ground (Node 0).\n");
                printf("Enter + terminal node (1 to %d): ", nodes - 1);
                posNode = 0;
                while (scanf("%d", &posNode) != 1)
                {
                    clearBuffer();
                    printf("Invalid input! Enter a node number (1 to %d): ", nodes - 1);
                }
                clearBuffer();
                while (posNode < 1 || posNode > nodes - 1)
                {
                    printf("Invalid Node! Enter again (1 to %d): ", nodes - 1);
                    while (scanf("%d", &posNode) != 1)
                    {
                        clearBuffer();
                        printf("Invalid input! Enter a node number (1 to %d): ", nodes - 1);
                    }
                    clearBuffer();
                }
                printf("Enter battery voltage (V): ");
                while (scanf("%lf", &volt) != 1)
                {
                    clearBuffer();
                    printf("Invalid input! Enter a number: ");
                }
                clearBuffer();
                break;
            }
            case 3:
            {
                if (posNode == 0)
                {
                    printf("No Battery Added! Try Again\n");
                    break;
                }

                double Left_Grid[MAX_NODES][MAX_NODES];
                double Right_Side_List[MAX_NODES];
                double V[MAX_NODES] = {0.0};

                printf("\nCalculating Matrix...\n");

                buildMatrix(resist, nodes, posNode, volt, Left_Grid, Right_Side_List);

                printf("Matrix built successfully!\n\n");
                for (i = 1; i < nodes; i++)
                {
                    for (j = 1; j < nodes; j++)
                    {
                        printf("%8.4f\t", Left_Grid[i][j]);
                    }
                    printf(" |  %8.4f\n", Right_Side_List[i]);
                }

                if (solveGaussian(Left_Grid, V, Right_Side_List, nodes) == 0)
                {
                    printf("\nCircuit cannot be solved! Check for a floating node.\n");
                    break;
                }

                
                FILE *fp = NULL;
                int saveChoice = 0;

                printf("\nSave results to file?\n1. Yes\n2. No\nEnter choice: ");
                while (scanf("%d", &saveChoice) != 1 || (saveChoice != 1 && saveChoice != 2))
                {
                    clearBuffer();
                    printf("Invalid choice! Enter 1 or 2: ");
                }
                clearBuffer();

                if (saveChoice == 1)
                {
                    fp = fopen("results.txt", "w");
                }
                
                printf("\nNode Voltages:\n");
                printf("V0 = 0.0000 V (Ground)\n");
                if(fp != NULL) 
                {
                    fprintf(fp, "=== MINISPICE CIRCUIT RESULTS ===\n\nNode Voltages:\nV0 = 0.0000 V (Ground)\n");
                }

                for (i = 1; i < nodes; i++)
                {
                    printf("V%d = %.4f V\n", i, V[i]);
                    if(fp != NULL) fprintf(fp, "V%d = %.4f V\n", i, V[i]); 
                }

                double RCurrent, RPower;
                double TPower = 0;   
                double BCurrent = 0; 

                printf("\nResistor Currents and Power:\n");
                if(fp != NULL) fprintf(fp, "\nResistor Currents and Power:\n");

                for (int k = 0; k < count; k++)
                {
                    int a = Rlist[k].n1;
                    int b = Rlist[k].n2;

                    RCurrent = (V[a] - V[b]) / Rlist[k].value;
                    RPower = RCurrent * RCurrent * Rlist[k].value;
                    TPower += RPower;

                    printf("R%d (Node %d - Node %d, %.4f ohm): I = %.4f A, P = %.4f W\n",k + 1, a, b, Rlist[k].value, RCurrent, RPower);
                    
                    
                    if(fp != NULL) 
                    {
                        fprintf(fp, "R%d (Node %d - Node %d, %.4f ohm): I = %.4f A, P = %.4f W\n",k + 1, a, b, Rlist[k].value, RCurrent, RPower);
                    }

                    if (a == posNode)
                    {
                        BCurrent += RCurrent;
                    }
                    else if (b == posNode)
                    {
                        BCurrent -= RCurrent;
                    }
                }

                printf("\nBattery Current = %.4f A\n", BCurrent);
                printf("Battery Power   = %.4f W\n", volt * BCurrent);
                printf("Total Resistor Power = %.4f W\n", TPower);

                
                if(fp != NULL) 
                {
                    fprintf(fp, "\nBattery Current = %.4f A\n", BCurrent);
                    fprintf(fp, "Battery Power   = %.4f W\n", volt * BCurrent);
                    fprintf(fp, "Total Resistor Power = %.4f W\n", TPower);
                    
                    fclose(fp); 
                    printf("\n[SUCCESS] Results successfully saved to 'results.txt'!\n");
                } 
                else if (saveChoice == 1)
                {
                    printf("\n[ERROR] Could not create results.txt!\n");
                }
                break;
            }
            case 4:
            {
                printf("Exiting...\n");
                break;
            }
            }
        }
        else
        {
            printf("Enter a valid choice!\n");
            choice = 0;
        }
    } while (choice != 4);

    for (int l = 0; l < nodes; l++)
    {
        free(resist[l]);
    }
    free(resist);

    return 0;
}
