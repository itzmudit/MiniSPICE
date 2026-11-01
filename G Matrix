// Top par ye zaroor check kar lena ki #define MAX_NODES 10 likha ho

void buildMatrix(double resist[][10], int nodes, int posNode, double volt, double Left_Grid[][10], double Right_Side_List[]) {
    int i, j;
    
    // Pehle saare dabbo ko 0 set karte hain (safai)
    for (i = 1; i < nodes; i++) {
        Right_Side_List[i] = 0.0;
        for (j = 1; j < nodes; j++) {
            Left_Grid[i][j] = 0.0;
        }
    }
    
    // Asli logic (If-Else)
    for (i = 1; i < nodes; i++) {
        
        // Agar ye battery wala node hai (Yahan voltage ki equation banegi)
        if (i == posNode) {
            Left_Grid[i][i] = 1.0;          // 1 * V
            Right_Side_List[i] = volt;      // = 10 (Battery ka direct voltage)
        }
        
        // Agar ye normal node hai (Yahan KCL yani 1/R ki equation banegi)
        else {
            for (j = 0; j < nodes; j++) {
                if (i != j && resist[i][j] > 0.0) {
                    
                    double g_val = 1.0 / resist[i][j]; // 1/R nikala
                    
                    Left_Grid[i][i] = Left_Grid[i][i] + g_val; // Khud ke dabbe mein plus
                    
                    if (j != 0) { // Agar dusra node ground nahi hai, toh wahan minus
                        Left_Grid[i][j] = Left_Grid[i][j] - g_val;
                    }
                }
            }
            // Right_Side_List[i] pehle se 0.0 hai, jo KCL ke hisaab se theek hai
        }
    }
}

-----------------------------------------------------------------------------

case 3:
        {
            // Ye do naye arrays banaye jo hamari equations ko store karenge
            double Left_Grid[10][10];
            double Right_Side_List[10]; 
            
            printf("\nCalculating Matrix...\n");
            
            // Yahan humne apne function ko saara data de diya
            buildMatrix(resist, nodes, posNode, volt, Left_Grid, Right_Side_List);
            
            // Check karne ke liye ki matrix bani ya nahi, usko print karke dekh lete hain
            printf("Matrix successfully ban gayi hai!\n\n");
            for (int i = 1; i < nodes; i++) {
                for (int j = 1; j < nodes; j++) {
                    printf("%.4f\t", Left_Grid[i][j]);
                }
                printf(" |  %.4f\n", Right_Side_List[i]);
            }
            break;
        }
