#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <chrono>
#include <iostream>


using namespace std;


const int imageSize = 520;
const int templSize_high = 80;
const int templSize_width = 80;
const int znccSize = imageSize;
const int N = templSize_high*templSize_width;



struct buf{
	double sum=0;
	double sum_square=0;
	double templ_mult=0;
	double col =0;
	double col_square=0;
};

void templ_row_sum(float t[], buf *B){
	
	for(int x=0;x<templSize_width;++x){
		B->sum += t[x];
		B->sum_square += pow(t[x],2);
	}
	
}

void row_sum(float m[],float t[], buf *mask_buf, int index){

	for(int x=0;x<templSize_width;++x){
		mask_buf->sum+=m[x+index];
		mask_buf->sum_square+=pow(m[x+index],2);
		mask_buf->templ_mult+=(m[x+index]*t[x]);
		
	}
	
}

void row_maskTempl_mult(float m[],float t[], buf *mask_buf, int index){    
		for(int x=0;x<templSize_width;++x){
			mask_buf->templ_mult+=(m[x+index]*t[x]);
		}
}


void row_maskTempl_mult_border(float m[],float t[], buf *mask_buf, int index){    
		for(int x=0;x<(templSize_width-(index-imageSize-templSize_width));++x){
			mask_buf->templ_mult+=(m[x+index]*t[x]);
		}
}



void zncc(float zncc_matrix[][znccSize],float img[imageSize][imageSize], float templ[templSize_high][templSize_width], float B, float D){
  
   
	
	float zncc_score;

	for(int x=0;x<znccSize;++x){
		int first=1;
		buf mask_buf;
		for(int y=0;y<znccSize;++y){
			if(x<imageSize-templSize_high){
				//no modification
				if(y<imageSize-templSize_width){
					if(first){
						for(int j=0;j<templSize_high;++j){
							row_sum(img[x+j],templ[j], &mask_buf,y);
							mask_buf.col += img[x+j][y];
							mask_buf.col_square += pow((img[x+j][y]),2);
						}
						first = 0;
					}
					else{
						mask_buf.sum -= mask_buf.col;
						mask_buf.sum_square -= mask_buf.col_square;
						mask_buf.templ_mult=0;
						mask_buf.col_square=0;
						mask_buf.col=0;
						double last_col =0;
						double last_col_square =0;
						for(int j=0;j<templSize_high;++j){
							row_maskTempl_mult(img[x+j],templ[j], &mask_buf, y);
							mask_buf.col += img[x+j][y];
							last_col += img[x+j][y+templSize_width-1];
							mask_buf.col_square += pow((img[x+j][y]),2);
							last_col_square += pow((img[x+j][y+templSize_width-1]),2);

						}
						mask_buf.sum += last_col;
						mask_buf.sum_square+= last_col_square;
					}
				}
				//approaching rigth border
				else{
					mask_buf.sum -= mask_buf.col;
					mask_buf.sum_square -= mask_buf.col_square;
					mask_buf.templ_mult=0;
					mask_buf.col_square=0;
					mask_buf.col=0;
					for(int j=0;j<templSize_high;++j){
						row_maskTempl_mult_border(img[x+j],templ[j], &mask_buf, y);
						mask_buf.col += img[x+j][y];
						mask_buf.col_square += pow((img[x+j][y]),2);
					}
					
				}
			}
			//approaching bottom border
			else{
				//bottom border inside image
				if(y<imageSize-templSize_width){
					if(first){
						for(int j=0;j<templSize_high-(x-imageSize-templSize_high);++j){
							row_sum(img[x+j],templ[j], &mask_buf,y);
							mask_buf.col += img[x+j][y];
							mask_buf.col_square += pow((img[x+j][y]),2);
						}
						first = 0;
					}
					else{
						mask_buf.sum -= mask_buf.col;
						mask_buf.sum_square -= mask_buf.col_square;
						mask_buf.templ_mult=0;
						mask_buf.col_square=0;
						mask_buf.col=0;
						double last_col =0;
						double last_col_square =0;
						for(int j=0;j<templSize_high-(x-imageSize-templSize_high);++j){
							row_maskTempl_mult(img[x+j],templ[j], &mask_buf, y);
							mask_buf.col += img[x+j][y];
							last_col += img[x+j][y+templSize_width-1];
							mask_buf.col_square += pow((img[x+j][y]),2);
							last_col_square += pow((img[x+j][y+templSize_width-1]),2);

						}
						mask_buf.sum += last_col;
						mask_buf.sum_square+= last_col_square;
					}
				}
				//bottom border + right border
				else{
					if(first){
						for(int j=0;j<templSize_high-(x-imageSize-templSize_high);++j){
							row_sum(img[x+j],templ[j], &mask_buf,y);
							mask_buf.col += img[x+j][y];
							mask_buf.col_square += pow((img[x+j][y]),2);
						}
						first = 0;
					}
					else{
						mask_buf.sum -= mask_buf.col;
						mask_buf.sum_square -= mask_buf.col_square;
						mask_buf.templ_mult=0;
						mask_buf.col_square=0;
						mask_buf.col=0;
						
						for(int j=0;j<templSize_high-(x-imageSize-templSize_high);++j){
							row_maskTempl_mult_border(img[x+j],templ[j], &mask_buf, y);
							mask_buf.col += img[x+j][y];
							mask_buf.col_square += pow((img[x+j][y]),2);
							
						}
						
					}
				}
			}

			zncc_score=((N*mask_buf.templ_mult)-(B*mask_buf.sum))/(D*(sqrt((N*mask_buf.sum_square - pow(mask_buf.sum ,2)))));
            zncc_matrix[x][y] = zncc_score;
		}
	}
	
}


int main(){
	// acquire image from file 
	float img[imageSize][imageSize];
	float templ[templSize_high][templSize_width];
	float zncc_matrix[znccSize][znccSize];

	
	ifstream f("C:/Users/edoar/Downloads/image2.txt");
	
	for(int i=0; i< imageSize; ++i){
		for(int j=0; j<imageSize; ++j){
			
			f >> img[i][j];
		}
	}
	
	ifstream t("C:/Users/edoar/Downloads/template2.txt");
	for(int i=0; i< templSize_high; ++i){
		for(int j=0; j<templSize_width; ++j){
			
			t >> templ[i][j];
		}
	}
   
	buf B;
	for(int x=0; x<templSize_high;++x){    
		templ_row_sum(templ[x],&B);
	}
   
	float D = sqrt((N*B.sum_square - pow(B.sum,2)));

	//stupid test
	

	//calculate the zncc score of the image value between [-1,1]
	printf("inizio a calcolare la zncc! \n");
	auto t1 = chrono::high_resolution_clock::now();
	zncc(zncc_matrix,img,templ, B.sum,D);
	auto t2 = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<chrono::seconds>( t2 - t1 );
	cout << "duration: " << duration.count() <<"s"<< endl; 

	printf(" value: %f\n",zncc_matrix[0][0]);
	printf(" value: %f\n",zncc_matrix[240][240]);
	printf(" value: %f\n",zncc_matrix[439][439]);
	printf(" value: %f\n",zncc_matrix[519][519]);
	for(int x=0;x<znccSize;++x){
		for(int y =0; y<znccSize;++y){
			if((zncc_matrix[x][y])<-1){
				printf("error pos [%d][%d]: value: %f\n",x,y,zncc_matrix[x][y]);
			}
		}
	}
	
}