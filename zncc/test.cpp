#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#define ISIZE 520
#define TSIZE 80
#define RSIZE 440



void mult_hd(float *img, float *templ, float*res);

int main(){
    bool correct = true;
    float *img_buf = (float*)malloc(sizeof(float)*ISIZE*ISIZE);
    float *templ_buf= (float*)malloc(sizeof(float)*TSIZE*TSIZE);
    float *res_buf_hd= (float*)malloc(sizeof(float)*RSIZE*RSIZE);
    float res_buf_sw[RSIZE][RSIZE];

    for(int i=0;i<ISIZE;++i){
        for(int j=0;j<ISIZE;++j){
            img_buf[i*ISIZE +j]= static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        }
    }

    for(int i=0;i<TSIZE;++i){
        for(int j=0;j<TSIZE;++j){
            templ_buf[i*TSIZE +j]= static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        }
    }

    for(int i=0; i<RSIZE;++i){
        for(int j=0;j<RSIZE;++j){
            for(int r=0;r<TSIZE;++r){
                for(int c=0;c<TSIZE;++c){
                    res_buf_sw[i][j] += img_buf[i*ISIZE +j]*templ_buf[i*TSIZE +j];
                }
            }
        }
    }

    mult_hd(img_buf,templ_buf,res_buf_hd);

    for(int i=0; i<RSIZE;++i){
        for(int j=0;j<RSIZE;++j){
            if(res_buf_hd[i*RSIZE +j] != res_buf_sw[i][j]){
                correct = false;
            }
        }
    }
    
    free(img_buf);
    free(templ_buf);
    free(res_buf_hd);

   if (correct) {
        printf("Test successful\n");
        return 0;
    }
     else {
        printf("Test unsuccessful\n");
        return -1;
    }

}