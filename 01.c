#include <stdio.h>
#include <stdlib.h>

int main()
{   double **resist, **volt;
    int n;

    printf("Enter the total number of nodes (Including ground node): ");
    scanf("%d",&n);
    
   
  

    resist = (double **) malloc(n * sizeof(double *));
    volt   = (double **) malloc(n * sizeof(double *));

    for (int i = 0; i < n; i++) 
    {

        resist[i] = (double *) calloc(n, sizeof(double));
        volt[i]   = (double *) calloc(n, sizeof(double));
    }

    int i,j;
    int choice=0;
    char input[50];
    do
    { 
        printf("Select:  \n"); //select Which component to input value for: 
        printf("1. Resistance \n2. Battery\n3. Exit\n");
        printf("Enter choice: ");
        scanf("%s",input);
        if(input[1] == '\0' && input[0] >= '1' && input[0] <= '3')
       {
            choice = input[0] - '0';
            switch(choice)
         {
            case 1:
            {
                for(i=0; i<n; i++)
                {
                    for(j=i+1; j<n; j++)
                    {
                        printf("Enter the value of resistor between Node %d and Node %d : ",i,j);
                        scanf("%lf",&resist[i][j]);
                        resist[j][i] = resist[i][j];
                    }
                }

                break;

            }
            case 2:
            {
                for(i=0; i<n; i++)
                {
                    for(j=i+1; j<n; j++)
                    {
                        printf("Enter the value of voltage between Node %d and Node %d :",i,j);
                        scanf("%lf",&volt[i][j]);
                        volt[j][i] = volt[i][j];
                    }
                }

                break;
            }
            case 3:
            {
                printf("Exiting...\n");
                break;
            }
            default:
            {
                printf("Enter a valid choice!\n");
                break;
            }
        }
       
    }
    else
    {
        printf("Enter a valid choice!\n");
        choice = 0;
    }
}while(choice!=3);
   


    return 0;
}
