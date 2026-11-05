#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define MAX_NODES 10

void clearBuffer()
{
    while (getchar() != '\n')
        ;
}

// resist is double ** because it is allocated with malloc/calloc in main()
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

void solveGaussian(double G_LHS[][MAX_NODES], double V[], double RHS[], int NoDe)
{
    // STEP 1: Forward Elimination
    int a, b, c;
    for (a = 1; a < NoDe - 1; a++)
    {
        for (b = a + 1; b < NoDe; b++)
        {
            if (G_LHS[a][a] == 0.0)
            {
                continue;
            }
            double factor = G_LHS[b][a] / G_LHS[a][a];
            for (c = a; c < NoDe; c++)
            {
                G_LHS[b][c] = G_LHS[b][c] - factor * G_LHS[a][c];
            }
            RHS[b] = RHS[b] - factor * RHS[a];
        }
    }

    // STEP 2: Back Substitution
    for (b = NoDe - 1; b >= 1; b--)
    {
        V[b] = RHS[b];
        for (c = b + 1; c < NoDe; c++)
        {
            V[b] = V[b] - (G_LHS[b][c] * V[c]);
        }
        if (G_LHS[b][b] != 0)
        {
            V[b] = V[b] / G_LHS[b][b];
        }
    }

    V[0] = 0.0; // ground
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

    // calloc sets every value to 0.0 (0 means no resistor)
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
                    printf("Enter 0 if no resistor is connected.\n");
                    for (i = 0; i < nodes; i++)
                    {
                        for (j = i + 1; j < nodes; j++)
                        {
                            int more = 0;
                            double r;
                            double total_g = 0; // sum of 1/R of all resistors in this pair

                            do
                            {
                                do
                                {
                                    printf("Enter the value of resistor between Node %d and Node %d : ", i, j);
                                    scanf("%lf", &r);
                                    clearBuffer();

                                    if (r < 0)
                                    {
                                        printf("Resistance cannot be negative! Enter again.\n");
                                    }
                                } while (r < 0);

                                more = 0;
                                if (r > 0)
                                {
                                    total_g = total_g + 1.0 / r; // parallel: conductances add

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
                                    scanf("%d", &more);
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

                            // store the equivalent resistance (0 means no resistor)
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
                    scanf("%d", &posNode);
                    clearBuffer();
                    while (posNode < 1 || posNode > nodes - 1)
                    {
                        printf("Invalid Node! Enter again (1 to %d): ", nodes - 1);
                        scanf("%d", &posNode);
                        clearBuffer();
                    }
                    printf("Enter battery voltage (V): ");
                    scanf("%lf", &volt);
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

                    solveGaussian(Left_Grid, V, Right_Side_List, nodes);

                    printf("\nNode Voltages:\n");
                    printf("V0 = 0.0000 V (Ground)\n");
                    for (i = 1; i < nodes; i++)
                    {
                        printf("V%d = %.4f V\n", i, V[i]);
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
