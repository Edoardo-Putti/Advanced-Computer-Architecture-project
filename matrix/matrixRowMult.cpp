
#include <stdio.h>
#include <stdlib.h>


#define ROW 80
#define COL 160


void mult_hd(float img[ROW][COL], float templ[ROW][ROW], float res[ROW][COL]){
  for(int x=0;x<80;++x){
      for(int y=0;y<80;++y){
          res[x][y]= img[x][y]*templ[x][y];
      }
  }

   for(int x=0;x<80;++x){
      for(int y=80;y<160;++y){
          res[x][y]= img[x][y]*templ[x][y-80];
      }
  }
}