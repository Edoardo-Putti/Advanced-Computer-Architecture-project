#include "string.h"
#include "ap_int.h"
#include "math.h"
#include <hls_stream.h>

#define ROW 80
#define COL 80
#define N ROW*COL



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

    /*for(int i=0;i<80;i++){
    #pragma HLS UNROLL
        tmp[i]=data[i];
    }*/
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
	ap_uint<512> tmp1[5];
#pragma HLS ARRAY_PARTITION variable=tmp1 complete
	ap_uint<512> tmp2[5];
#pragma HLS ARRAY_PARTITION variable=tmp2 complete
	ap_uint<512> tmp3[5];
#pragma HLS ARRAY_PARTITION variable=tmp3 complete

	float r_sum[158];
#pragma HLS ARRAY_PARTITION variable=r_sum complete
	static float lmask[ROW][COL];
#pragma HLS ARRAY_PARTITION variable=lmask cyclic factor=32 dim=1
	static float lt[ROW][COL];//
#pragma HLS ARRAY_PARTITION variable=lt cyclic factor=32 dim=1
	float lcol[COL];
#pragma HLS ARRAY_PARTITION variable=lcol complete
	float D, B;

	float txm_acc[80];
#pragma HLS ARRAY_PARTITION variable=txm_acc cyclic factor=10 dim=1
	float sum_acc[80];
#pragma HLS ARRAY_PARTITION variable=sum_acc cyclic factor=10 dim=1
	float pow_acc[80];
#pragma HLS ARRAY_PARTITION variable=pow_acc cyclic factor=10 dim=1

	static float txm=0;
	static float sum=0;
	static float pow2=0;
	static float f_col=0;
	static float f_pow=0;

	data_struct out_data;
	D=d;
	B=b;

if(init){
	//copy the template in the FPGA BRAMs only once for the all process
	for(int r=0;r<ROW;r++){
#pragma HLS PIPELINE
		std::cout << "start memcpy template" << std::endl;
		memcpy(&tmp1[0],&t[r*5],sizeof(ap_uint<512>)*5);
		for(int c=0;c<5;c++){
//#pragma HLS UNROLL
			 ap_uint<512> a= tmp1[c];
			for(int j=0;j<16;j++){
#pragma HLS UNROLL
				int upper = (j+1)*32-1;
				int lower = j*32;

				unsigned int t_tmp =a.range(upper,lower);

				float t_val = *((float *)&t_tmp);
				std::cout << "coping val: "<< t_val << std::endl;
				lt[r][(c*16)+j]=t_val;
			}
		}
	}
	//memcpy(lt,t,sizeof(float)*N);

}

if(new_row){
	//copy the new mask this procedure needs to be done for each new row of the img
	sum=0;
	pow2=0;
	txm=0;
	f_col=0;
	f_pow=0;

	//memcpy(lmask,mask,sizeof(float)*N);


	for(int r=0;r<ROW;r++){
#pragma HLS PIPELINE
		memcpy(&tmp2[0],&mask[r*5],sizeof(ap_uint<512>)*5);
		for(int c=0;c<5;c++){
//#pragma HLS UNROLL
			ap_uint<512> a=tmp2[c];
			for(int j=0;j<16;j++){
#pragma HLS UNROLL
				int upper = (j+1)*32-1;
				int lower = j*32;

				unsigned int m_tmp = a.range(upper,lower);
				float m_val = *((float *)&m_tmp);
				lmask[r][(c*16)+j]=m_val;
			}
		}
	}

	for(int x=0;x<80;x++){
#pragma HLS UNROLL factor=10
		txm_acc[x] = 0;
		sum_acc[x] = 0;
		pow_acc[x] = 0;
	}

	for(int r=0; r<ROW;r++){
	#pragma HLS PIPELINE

		f_col += lmask[r][0];;
		f_pow += lmask[r][0]*lmask[r][0];

		for(int c=0; c<80;c++){
		#pragma HLS UNROLL factor=10

			txm_acc[c] += (lt[r][c])*(lmask[r][c]);
			sum_acc[c] += lmask[r][c];
			pow_acc[c] += lmask[r][c]*lmask[r][c];

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

memcpy(tmp3,mask,sizeof(ap_uint<512>)*5);
	for(int c=0;c<5;c++){
//#pragma HLS UNROLL
		ap_uint<512> a=tmp3[c];
		for(int j=0;j<16;j++){
#pragma HLS UNROLL
			int upper = (j+1)*32-1;
			int lower = j*32;

			unsigned int m_tmp = a.range(upper,lower);
			float m_val = *((float *)&m_tmp);
			lcol[(c*16)+j]=m_val;
		}
	}

	//memcpy(lcol,mask,sizeof(float)*COL);

	for(int r=0;r<ROW;++r){
#pragma HLS PIPELINE
		for(int c=1;c<COL;c++){

			lmask[r][c-1]=lmask[r][c];
		}
		lmask[r][COL-1]=lcol[r];

	}


	for(int r=0;r<ROW;r++){
	#pragma HLS PIPELINE

		f_col += lmask[r][0];
		f_pow += lmask[r][0]*lmask[r][0];
		sum+=lcol[r];
		pow2+=lcol[r]*lcol[r];

		for(int c=0;c<COL;c++){//9
#pragma HLS UNROLL factor=10
			txm_acc[c]+=lmask[r][c]*lt[r][c];


		}

	}

		txm = reducesum(txm_acc,r_sum);

}

out_data.data=((N*txm)-(B*sum))/(D*(sqrt((N*pow2 - (sum*sum)))));
out_data.last=finish;
out.write(out_data);
}
