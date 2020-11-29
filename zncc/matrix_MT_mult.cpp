
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ISIZE 520
#define TSIZE 80
#define RSIZE 440

void mult_hd(float *img, float *templ, float*res){
    float img_buf[ISIZE][ISIZE];
    float templ_buf[TSIZE][TSIZE];
    float res_buf[RSIZE][RSIZE];

    memcpy(&img_buf,img, sizeof(float)*ISIZE*ISIZE);
    memcpy(&templ_buf,templ, sizeof(float)*TSIZE*TSIZE);

    for(int i=0; i<RSIZE;++i){
        for(int j=0;j<RSIZE;++j){
            for(int r=0;r<TSIZE;++r){
                for(int c=0;c<TSIZE;++c){
                    res_buf[i][j] += img_buf[r+i][c+j]*templ_buf[r][c];
                }
            }
        }
    }


    memcpy(res,&res_buf[0][0],sizeof(float)*RSIZE*RSIZE);
}