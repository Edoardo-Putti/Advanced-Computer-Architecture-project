
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <chrono>
#include <iostream>

using namespace std;


const int imageSize = 520;
const int templSize = 80;
const int znccSize = 440;
const int N = templSize*templSize;



struct buf{
    float sum=0;
    float sum_square=0;
    float templ_mult=0;
   
};

void templ_row_sum(float t[], buf *B){
    
    for(int x=0;x<templSize;++x){
        B->sum += t[x];
        B->sum_square += pow(t[x],2);
    }
    
}

void row_sum(float m[],float t[], buf *mask_buf, int index){

    for(int x=0;x<templSize;++x){
        mask_buf->sum+=m[x+index];
        mask_buf->sum_square+=pow(m[x+index],2);
        mask_buf->templ_mult+=(m[x+index]*t[x]);
        
    }
    
}

void row_maskTempl_mult(float m[],float t[], buf *mask_buf, int index){
    
    for(int x=0;x<templSize;++x){
        mask_buf->templ_mult+=(m[x+index]*t[x]);
    }

}




void zncc(float zncc_matrix[][znccSize],float img[imageSize][imageSize], float templ[templSize][templSize], float B, float D){
  
   
    
    float zncc_score;

    for(int x=0;x<znccSize;++x){
        
        
        for(int y=0;y<znccSize;++y){
            zncc_score =0;
            buf mask_buf;
                
                for(int j=0;j<templSize;++j){
                    row_sum(img[x+j],templ[j], &mask_buf,y);
                    
                }
               
           
            
            zncc_score=((N*mask_buf.templ_mult)-(B*mask_buf.sum))/(D*(sqrt((N*mask_buf.sum_square - pow(mask_buf.sum ,2)))));
            zncc_matrix[x][y] = zncc_score;

        }
        //printf("finish row %d\n",x);
    }
    
}


int main(){
    // acquire image from file 
    float img[imageSize][imageSize];
    float templ[templSize][templSize];
    float zncc_matrix[znccSize][znccSize];

    ifstream f("C:/Users/edoar/Downloads/image2.txt");
    
    for(int i=0; i< imageSize; ++i){
        for(int j=0; j<imageSize; ++j){
            f >> img[i][j];
        }
    }

    ifstream t("C:/Users/edoar/Downloads/template2.txt");
    for(int i=0; i< templSize; ++i){
        for(int j=0; j<templSize; ++j){
            t >> templ[i][j];
        }
    }
    //sta\rt calculating B
    buf B;
    for(int x=0; x<templSize;++x){    
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
    
    
}