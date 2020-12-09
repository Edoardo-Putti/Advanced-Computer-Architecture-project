#include <math.h>
#include <stdio.h>
#include <stdlib.h>


#define ROW 80
#define COL 80
#define N ROW*COL

void zncc_hd(float* img, float* templ, float* res, bool first, float B, float D){
#pragma HLS INTERFACE s_axilite port=return
#pragma HLS INTERFACE s_axilite port=D
#pragma HLS INTERFACE s_axilite port=B
#pragma HLS INTERFACE s_axilite port=first
#pragma HLS INTERFACE m_axi depth=6400 port=templ
#pragma HLS INTERFACE m_axi depth=6400 port=img
#pragma HLS INTERFACE m_axi depth=6400 port=res

    //static float mask2[ROW][COL]
    static float mask[ROW][COL];
    static float templ_buf[ROW][COL];
    //static float zncc[R][C];

    //maybe use a single array to store all this values
    static float mask_pixel_sum=0;
    static float mask_pixel_square=0;
    static float prod_m_t=0;
    static float first_col=0;
    static float first_col_square=0;
    static float last_col=0;
    static float last_col_square=0;
    static int zncc_index=0;

if(first){
/*
In these first two loops we save the first column of the mask passed,
the local mask is initialized and the sums/squares of its pixel are calculated
*/
	mask_pixel_sum = 0;
	mask_pixel_square = 0;
	prod_m_t = 0;
	first_col=0;
	first_col_square=0;

  for(int x=0;x<ROW;++x){
      first_col += img[x*COL];
      first_col_square += pow(img[x*COL],2);
      for(int y=0;y<COL;++y){
          mask[x][y] = img[x*COL+y];
          mask_pixel_sum += img[x*COL+y];
          mask_pixel_square += pow(img[x*COL+y],2);
      }
  }

//the local template is initialized
   for(int x=0;x<ROW;++x){
      for(int y=0;y<COL;++y){
          templ_buf[x][y] = templ[x*COL+y];
      }
  }

//the product pixel for pixel bettween the mask and the template is calculated
  for(int x=0;x<ROW;++x){
      for(int y=0;y<COL;++y){
          prod_m_t += templ_buf[x][y]*mask[x][y];
      }
  }

/*
in case there are still resources available we can duplicate the mask and start the
shifting so when the next data are ready we just need to add the new column

    for(int x=0;x<80;++x){
        for(int y=0;y<80;++y){
            mask2[x][y]= img[x*COL+y];
        }
    }

    for(int r = 0; r<ROW; ++r){
        for(int c=1;c<COL;++c){
            mask2[r][c-1]=mask2[r][c];
        }
    }
*/
}
else{
    //shift all elements
    for(int r = 0; r<ROW; ++r){
        for(int c=1;c<COL;++c){
            mask[r][c-1]=mask[r][c];
        }
    }
    mask_pixel_sum -= first_col;
    mask_pixel_square -= first_col_square;
    first_col=0;
    first_col_square=0;
    last_col=0;
    last_col_square=0;
    //add new column and calculate new first and last col
    for(int r =0; r<ROW;++r){
        mask[r][COL-1]=img[r];
        last_col += img[r];
        last_col_square += pow(img[r],2);
        first_col += mask[r][0];
        first_col_square += pow(mask[r][0],2);
    }

    //calculate the new sum/square of the mask
    mask_pixel_sum += last_col;
    mask_pixel_square += last_col_square;

    /*the product pixel for pixel bettween the mask and the template is
    first set to zero in order to calculate the one with the new mask
    */
    prod_m_t = 0;
    for(int x=0;x<ROW;++x){
        for(int y=ROW;y<COL;++y){
            prod_m_t += templ_buf[x][y]*mask[x][y];
        }
    }
    mask_pixel_sum -= first_col;
    mask_pixel_square -= first_col_square;

}

//save back the result for this mask
res[zncc_index] = ((N*prod_m_t)-(B*mask_pixel_sum))/(D*(sqrt((N*mask_pixel_square - pow(mask_pixel_sum,2)))));
zncc_index++;

}
