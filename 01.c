#include <stdio.h>
#include <stdlib.h>
#include <string.h>   // TODO: use nahi ho raha, hata do
#include<math.h>

    void clearBuffer() // CHANGE 1: naya function
    {
       while (getchar() != '\n');
    }
// TODO: "CHANGE 1" comment aur extra indentation hatao
// TODO: getchar() ko int variable mein lo aur '\n' ke saath EOF bhi check karo, warna EOF pe infinite loop

int main()
{   double **resist, volt=0;
    int posNode=0;
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


    resist = (double **) malloc(nodes * sizeof(double *));

    if(resist==NULL)
    {
        printf("Memory allocation Failed.\n");
        return 1;
    }


   for (int i = 0; i < nodes; i++)
    {
        resist[i] = (double *) calloc(nodes, sizeof(double));
        if (resist[i] == NULL)
        {
            printf("Memory allocation failed.\n");
            for(int l=0;l<i;l++)
            {
                free(resist[l]);
            }
            free(resist);
            return 1;
        }
    }



    int i,j;
    int choice=0;
    char input[50];
do
{
    printf("Select:  \n"); //select Which component to input value for:
    printf("1. Resistance \n2. Battery\n3. Solve Circuit\n4. Exit\n");
    printf("Enter choice: ");
    scanf("%49s",input);
    if(input[1] == '\0' && input[0] >= '1' && input[0] <= '4')
    {
        choice = input[0] - '0';
        switch(choice)
        {   
            case 1:
            {
                printf("Enter 0 if no resistor is connected.\n");
                for(i=0; i<nodes; i++)
                {
                    for(j=i+1; j<nodes; j++)
                    {
                        printf("Enter the value of resistor between Node %d and Node %d : ",i,j);   
                        scanf("%lf",&resist[i][j]);
                        // TODO: clearBuffer() yahan
                        // TODO: scanf fail ho ya value negative ho to dobara poocho
                        resist[j][i] = resist[i][j];
                    }
                }
                break;
            }
            case 2:
            {
                printf("Negative (-) terminal of battery is connected to ground (Node 0).\n");
                printf("Enter + terminal node (1 to %d): ", nodes - 1);
                posNode=0;
                scanf("%d", &posNode);
                // TODO: clearBuffer() yahan
                while(posNode<1||posNode>nodes-1)
                {
                    printf("Invalid Node! Enter again (1 to %d): ",nodes-1);
                    scanf("%d",&posNode);
                    // TODO: clearBuffer() yahan (iske bina "abc" daalne pe infinite loop)
                } 
                printf("Enter battery voltage (V): ");
                scanf("%lf",&volt);
                // TODO: clearBuffer() yahan
                break;
            }
            case 3:
            {
                // TODO: Solve ka code yahan aayega:
                //   - check: battery daali hai (posNode != 0), koi node floating nahi
                //   - resist se G matrix banao (ground chhod ke)
                //   - battery wale node ki equation set karo
                //   - Gaussian elimination (alag function mein)
                //   - node voltages, current, power print karo
                //   - results.txt mein save karo
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
}while(choice!=4);

for(int l=0;l<nodes;l++)
{
    free(resist[l]);
}
free(resist);

    return 0;
}
