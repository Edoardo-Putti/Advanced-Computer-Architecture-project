#include <stdio.h>
#include <stdlib.h>
#include "signals.h"

#define ROW 80
#define COL 160


void mult_hd(float img[ROW][COL], float templ[ROW][ROW], float res[ROW][COL]);

int main(){
    bool correct = true;
    float res[ROW][COL];

   mult_hd(image,templ,res);
    
    
    for(int x=0; x<ROW; ++x){
        for(int y=0; y<COL; ++y){
            if(res[x][y]!=gold[x][y]){
                correct=false;
            }
        }
    }

   if (correct) {
        printf("Test successful\n");
        return 0;
    }
     else {
        printf("Test unsuccessful\n");
        return -1;
    }

}