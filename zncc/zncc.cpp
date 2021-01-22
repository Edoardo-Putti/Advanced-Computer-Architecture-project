#include "string.h"
#include "ap_int.h"
#include "math.h"
#include <hls_stream.h>

#define ROW 80
#define COL 80
#define N ((ROW*COL)/16)
#define N_AP (ROW/16)

struct data_struct{
  float data;
  bool last;
};



void zncc(ap_uint<512> *mask,ap_uint<512> *t, hls::stream<data_struct> &out, float b, float d, bool new_row, bool init, bool end){
//interfaces for the axi master communication
#pragma HLS INTERFACE m_axi depth=6400 port=mask bundle=gmem0
#pragma HLS INTERFACE m_axi depth=6400 port=t bundle=gmem1
#pragma HLS INTERFACE m_axi depth=6400 port=res bundle=gmem2

//interfaces for the addresses of the variables
#pragma HLS INTERFACE s_axilite port=mask
#pragma HLS INTERFACE s_axilite port=t
#pragma HLS INTERFACE s_axilite port=res
#pragma HLS INTERFACE s_axilite port=b
#pragma HLS INTERFACE s_axilite port=d
//interface for start/stop the kernel
#pragma HLS INTERFACE s_axilite port=return
//local variables
	static ap_uint<512> lmask[N], lt[N];//look for size
	float lcol[COL];
	float ld, lb;
	float txm_acc[16] = {};
	float sum_acc[16] = {};
	float pow_acc[16] = {};
	static float txm=0;
	static float sum=0;
	static float pow2=0;
	static float f_col=0;
	static float f_pow=0;
	float score[0];
	data_struct out_data;
	ld=d;
	lb=b; 
	if(init){
		//copy the template in the FPGA BRAMs only once for the all process
		memcpy(lt,t,sizeof(ap_uint<512>)*N);
	}
	if(new_row){
		//copy the new mask this procedure needs to be done for each new row of the img
		sum=0;
		pow2=0;
		txm=0;
		f_col=0;
		f_col=0;
		memcpy(lmask,mask,sizeof(ap_uint<512>)*N);
		loopRow:for(int r; r<ROW;r++){
		#pragma HLS PIPELINE

			unsigned int tmp = lmask[r*N_AP].range(31,0);
			float tmp_val = *((float *)&tmp);

			f_col += tmp_val;
			f_pow += pow(tmp_val,2);

			loopCol:for(int c; c<N_AP;c++){
			#pragma HLS PIPELINE

				loopElem:for(int j=0;j<16;j++){

					int upper = (j+1)*32-1;
					int lower = j*32;

					unsigned int m_tmp = lmask[r*N_AP+c].range(upper,lower);
					unsigned int t_tmp = lt[r*N_AP+c].range(upper,lower);

					float m_val = *((float *)&m_tmp);
					float t_val = *((float *)&t_tmp);

					txm_acc[j]+= t_val*m_val;
					sum_acc[j]+= m_val;
					pow_acc[j]+= pow(m_val,2);

				}
			}
		}
		for(int i=0;i<16;i++){
		#pragma HLS PIPELINE
			sum += sum_acc[i];
			pow2 += pow_acc[i];
			txm += txm_acc[i];	
		}
	}
	else{
		sum -= f_col;
		pow2 -= f_pow;
		txm=0;
		f_col=0;
		f_pow=0;

		memcpy(lcol,mask,sizeof(ap_uint<512>)*N_AP);

		for(int r=0;r<ROW;r++){

			unsigned int tmp = lmask[r*N_AP].range(63,32);
			float tmp_val = *((float *)&tmp);

			f_col += tmp_val;
			f_pow += pow(tmp_val,2);
			sum += lcol[r];
			pow2 += pow(lcol[r],2);

			for(int c=0;c<N_AP;c++){
			#pragma HLS PIPELINE
				for(int j=1;j<16;j++){

					int upper = (j)*32-1;
					int lower = (j-1)*32;
					int upper2 = (j+1)*32-1;
					int lower2 = j*32;

					unsigned int m_1 = lmask[r*N_AP+c].range(upper2,lower2);
					unsigned int t_tmp = lt[r*N_AP+c].range(upper,lower);

					float m_val = *((float *)&m_1);
					float t_val = *((float *)&t_tmp);

					txm_acc[j]+= t_val*m_val;
					//shift
					lmask[r*N_AP+c].range(upper,lower) = m_1;

				}
				unsigned int m_1 = lmask[r*N_AP+c+1].range(31,0);
				lmask[r*N_AP+c].range(511,480) = m_1;
				lmask[r*N_AP+4].range(511,480)= *((unsigned int *)&lcol[r]);
				unsigned int t_tmp =lt[r*N_AP+4].range(511,480);
				float t_val = *((float *)&t_tmp);
				txm += t_val*lcol[r];
			}
			
		}
		for(int i=0;i<16;i++){
		#pragma HLS PIPELINE
			txm += txm_acc[i];	
		}
	}
	out_data.data=((ROW*COL*txm)-(b*sum))/(d*(sqrt((ROW*COL*pow2 - pow(sum,2)))));
	out_data.last=end;
	out.write(out_data);/*
	score[0]=((ROW*COL*txm)-(b*sum))/(d*(sqrt((ROW*COL*pow2 - pow(sum,2)))));
	memcpy(&out[0],&score[0],sizeof(float));*/
}
