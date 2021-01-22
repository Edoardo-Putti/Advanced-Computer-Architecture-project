#include <stdio.h>
#include <stdlib.h>
#include "signals.h"
#include <math.h>
#include "ap_int.h"
#include <hls_stream.h>

#define IMG_SIDE 520
#define TMP_SIDE 80
#define RES_SIDE (IMG_SIDE-TMP_SIDE+1)

struct data_struct{
  float data;
  bool last;
};

void zncc(ap_uint<512> *mask,ap_uint<512> *t, hls::stream<data_struct> &out, float b, float d, bool new_row, bool init, bool end);

int main(){
    bool correct = true;
    bool new_row = true;
    bool setup = true;
    bool end = false;
    float res[440][440];
    float* img=image;
    float* t=templ;
    float B = b;
    float D = d;
    float mask[TMP_SIDE*TMP_SIDE];
    float col[TMP_SIDE];
    float out[0];
    hls::stream<data_struct> stream_out("stream_out");
    float ka =666;
/*
    	ap_uint<512> test[25];
        float vec[160];
        for(int x=0;x<160;x++){
            vec[x]=(float)x;
        }
        float ta[80];
        memcpy(test,vec,sizeof(ap_uint<512>)*25);
        memcpy(ta,test,sizeof(ap_uint<512>)*5);

        for(int x=0;x<80;x++){
        	printf("%f\n",ta[x]);
        }

        for(int r=0;r<2;r++){
        	unsigned int tmp = test[r*5].range(63,32);
        	float tmp_val = *((float *)&tmp);
        	printf("--> %f \n",tmp_val);
        	int k=0;
            for(int c=0;c<5;c++){

            	for(int j=1;j<16;j++){

					int upper = (j)*32-1;
					int lower = (j-1)*32;
					int upper2 = (j+1)*32-1;
					int lower2 = j*32;

					//unsigned int m_0 = lmask[r*N_AP+c].range(upper,lower);
					unsigned int m_1 = test[r*5+c].range(upper2,lower2);

					//float m0 = *((float *)&m_0);
					float m1 = *((float *)&m_1);

					test[r*5+c].range(upper,lower) = m_1;
            	}
            	unsigned int m_1 = test[r*5+c+1].range(31,0);
            	test[r*5+c].range(511,480) = m_1;
            	//test[r*5+4].range(511,480) = *((unsigned int *)&ka);
            	for(int j=0;j<16;j++){

                    int up = (j+1)*32-1;
                    int low = j*32;
                    unsigned int t = test[r*5+c].range(up,low);
                    float t_val = *((float *)&t);
                    printf("%d %d : %f \n",r,k,t_val);
                    k++;
                }
            }
        }
*/
    for(int r=0; r<RES_SIDE; ++r){
        new_row = true;
        for(int x=0; x<TMP_SIDE;++x){
            for(int y=0; y<TMP_SIDE;++y){
                mask[x*TMP_SIDE+y] = img[(r+x)*IMG_SIDE+y];
            }
        }

        zncc((ap_uint<512>*)mask,(ap_uint<512>*)t,stream_out,B,D,new_row,setup,end);
        float tmp = stream_out.read().data;
        //res[r][0]=out[0];
        res[r][0]=tmp;
        //printf("%f\n",res[r][0]);
        setup=false;
        for(int c=1; c<RES_SIDE;++c){
            new_row=false;
            for(int x=0; x<TMP_SIDE;++x){
                col[x]=img[x*IMG_SIDE+IMG_SIDE*r+TMP_SIDE+(c-1)];
            }

            zncc((ap_uint<512>*)col,(ap_uint<512>*)t,stream_out,B,D,new_row,setup,end);
            res[r][c]=stream_out.read().data;
            //res[r][c]=out[0];
            printf("[%d,%d]-> %f\n",r,c,res[r][0]);
        }
    }


    
    for(int x=0; x<RES_SIDE-1 && correct; ++x){
        for(int y=0; y<RES_SIDE-1 && correct; ++y){
            if(abs(res[x][y])-abs(gold[x*RES_SIDE+y]) >0.0001){
            	printf("pos %d,%d res: %f | gold: %f\n",x,y,res[x*RES_SIDE+y],gold[x*RES_SIDE+y]);
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
