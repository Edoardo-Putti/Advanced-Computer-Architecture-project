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

float reducesum(float data[80], float tmp[158]){
#pragma HLS ALLOCATION instances=FAddSub_fulldsp core

#pragma INLINE
    int p3=40;
    int p1=0;
    int p2=80;

    memcpy(tmp,data,sizeof(float)*80);

    for(int i=0;i<=5;i++){
    #pragma HLS UNROLL
        for(int j=0;j<p3;j++){
        #pragma HLS UNROLL
            tmp[j+p2]=tmp[j*2+p1]+tmp[j*2+1+p1];

        }
        p1=p2;
        p2+=p3;
        p3=(p3/2);
    }
    return (tmp[154]+tmp[157]);
};


void zncc(ap_uint<512> *mask,ap_uint<512> *t, hls::stream<data_struct> &out, float b, float d, bool new_row, bool init,bool finish){
//pragmas to reduce the resources utilizations
#pragma HLS ALLOCATION instances=DDiv core
#pragma HLS ALLOCATION instances=DSqrt core
#pragma HLS ALLOCATION instances=fmul limit=2 operation
#pragma HLS ALLOCATION instances=fadd limit=2 operation
#pragma HLS ALLOCATION instances=reducesum limit=1 function
#pragma HLS ALLOCATION instances=FMul_maxdsp core
#pragma HLS ALLOCATION instances=FAddSub_fulldsp core
//interfaces for the axi master communication
#pragma HLS INTERFACE m_axi depth=6400 port=mask bundle=gmem0
#pragma HLS INTERFACE m_axi depth=6400 port=t bundle=gmem2
#pragma HLS INTERFACE axis port=out
#pragma HLS INTERFACE ap_ctrl_none port=return


//interfaces for the addresses of the variables
#pragma HLS INTERFACE s_axilite port=mask
#pragma HLS INTERFACE s_axilite port=t
#pragma HLS INTERFACE s_axilite port=b
#pragma HLS INTERFACE s_axilite port=d
#pragma HLS INTERFACE s_axilite port=finish
#pragma HLS INTERFACE s_axilite port=new_row
#pragma HLS INTERFACE s_axilite port=init
//interface for start/stop the kernel
#pragma HLS INTERFACE s_axilite port=return

//local variables
	static ap_uint<512> lmask[N], lt[N], lcol[5];//look for size
	float D, B;
	float r_sum[158];
#pragma HLS ARRAY_PARTITION variable=r_sum complete
	float txm_acc[80];
	float sum_acc[80];
	float pow_acc[80];
	static float txm;
	static float sum;
	static float pow2;
	static float f_col;
	static float f_pow;
	data_struct out_data;
	D=d;
	B=b;

	if(init){
		//copy the template in the FPGA BRAMs only once for the all process
		memcpy(lt,t,sizeof(ap_uint<512>)*N);
	}
	if(new_row){
		//copy the new mask this procedure needs to be done for each new row of the img
		memcpy(lmask,mask,sizeof(ap_uint<512>)*N);

		for(int x=0;x<80;x++){
		#pragma HLS UNROLL factor=10
				txm_acc[x] = 0;
				sum_acc[x] = 0;
				pow_acc[x] = 0;
			}

		loopRow:for(int r; r<ROW;r++){
		#pragma HLS PIPELINE

			unsigned int tmp = lmask[r*N_AP].range(31,0);
			float tmp_val = *((float *)&tmp);

			f_col += tmp_val;
			f_pow += tmp_val*tmp_val;

			loopCol:for(int c; c<N_AP;c++){
			#pragma HLS PIPELINE

				loopElem:for(int j=0;j<16;j++){

					int upper = (j+1)*32-1;
					int lower = j*32;

					unsigned int m_tmp = lmask[r*N_AP+c].range(upper,lower);
					unsigned int t_tmp = lt[r*N_AP+c].range(upper,lower);

					float m_val = *((float *)&m_tmp);
					float t_val = *((float *)&t_tmp);

					txm_acc[j*(c+1)]+= t_val*m_val;
					sum_acc[j*(c+1)]+= m_val;
					pow_acc[j*(c+1)]+= m_val*m_val;

				}
			}
		}

		sum = reducesum(sum_acc,r_sum);
		pow2 = reducesum(pow_acc,r_sum);
		txm = reducesum(txm_acc,r_sum);
	}
	else{
		sum -= f_col;
		pow2 -= f_pow;
		txm=0;
		f_col=0;
		f_pow=0;

		for(int x=0;x<80;x++){
		#pragma HLS UNROLL factor=10
			txm_acc[x] = 0;
		}

		memcpy(lcol,mask,sizeof(ap_uint<512>)*N_AP);
		float tmp_col[80];

		for(int c=0;c<5;c++){
			for(int j=0;j<16;j++){
				int upper = (j+1)*32-1;
				int lower = j*32;

				unsigned int m_tmp = lcol[c].range(upper,lower);

				tmp_col[c*16+j] = *((float *)&m_tmp);
			}
		}

		//shifter motherfuker
		int i=0;
		for(int r=0;r<ROW;r++){
			for(int c=0;c<4;c++){
				unsigned int last = lmask[r*N_AP+c+1].range(31,0);
				for(int j=1;j<16;j++){
					int second_up = (j+1)*32-1;
					int second_low = (j-1)*32;
					int first_up = (j)*32-1;
					int first_low = j*32;
					unsigned int to_shift = lmask[r*N_AP+c].range(second_up,second_low);
					lmask[r*N_AP+c].range(first_up,first_low) = to_shift;

				}
				lmask[r*N_AP+c].range(511,480) = last;
			}
			for(int j=1;j<16;j++){
				int second_up = (j+1)*32-1;
				int second_low = (j-1)*32;
				int first_up = (j)*32-1;
				int first_low = j*32;
				unsigned int to_shift = lmask[r*N_AP+4].range(second_up,second_low);
				lmask[r*N_AP+4].range(first_up,first_low) = to_shift;
			}
			float last = lcol[r];
			lmask[r*N_AP+4].range(511,480) =*((unsigned int *)&last);
		}

		for(int r=0;r<ROW;r++){
			unsigned int first = lmask[r*N_AP].range(31,0);
			float f_val = *((float *)&first);
			f_col +=  f_val;
			f_pow += f_val*f_val;

			for(int c=0;c<5;c++){
				for(int j=0;j<16;j++){
					int upper = (j+1)*32-1;
					int lower = j*32;

					unsigned int m_tmp = lmask[r*N_AP+c].range(upper,lower);
					unsigned int t_tmp = lt[r*N_AP+c].range(upper,lower);

					float m_val = *((float *)&m_tmp);
					float t_val = *((float *)&t_tmp);

					txm_acc[j*(c+1)]+= t_val*m_val;
				}
			}
		}

		txm = reducesum(txm_acc,r_sum);
	}
	out_data.data=((ROW*COL*txm)-(b*sum))/(d*(sqrt((ROW*COL*pow2 - sum*sum))));
	out_data.last=finish;
	out.write(out_data);
}
