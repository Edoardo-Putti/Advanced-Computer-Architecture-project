#include <stdio.h>
#include <stdlib.h>
#include "signals.h"

#define ROW 440
#define COL 440


void zncc_hd(float* img, float* templ, float* res, bool first, float B, float D);

int main(){
    bool correct = true;
    bool first = true;
    float res[440*440];
    float B = b;
    float D = d;
    float mask[80*80];
    float col[80];
    for(int r=0; r<ROW; ++r){
        first = true;
        for(int x=0; x<80;++x){
            for(int y=0; y<80;++y){
                mask[x*80+y] = image[x*520*r+y];
            }
        }
        zncc_hd(mask,templ,res,first,B,D);
        for(int c=1; c<COL;++c){
            first=false;
            for(int x=0; x<80;++x){
                col[x]=image[x*520+c*449];
            }
            zncc_hd(col,templ,res,first,B,D);
            
        }
    }

   
    
    
    for(int x=0; x<ROW; ++x){
        for(int y=0; y<COL; ++y){
            if(res[x*COL+y]!=gold[x][y]){
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