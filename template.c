#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "cpu_time.c"

#define binWidth    6000.0  /* the width of bin */
#define binHeight   3210.0  /* the height of bin */
#define binArea     binWidth*binHeight  /* the area of bin */
#define TIMELIM     300 /* the time limit for the algorithm in seconds */

/* data of BPP instance */
typedef struct{
  int *id;         /* id of rectangles*/
  int *height;     /* height of rectangles*/
  int *width;      /* width of rectangles*/
  int NumberofRect;       /* the number of rectangles */
  double areaofRects;     /* the area of rectagles */
} RectData;     /* data of the rectanle */

/* various data often necessary during the search */
typedef struct{
  double timebird;        /* the time before reading the instance data */
  double starttime;       /* the time the search started */
  double endtime;         /* the time the search ended */
  int *bestsolId;         /* the id of the best solution found so far */
  int *bestsolX;          /* the x-coordinate of the best solution found so far */
  int *bestsolY;          /* the y-coodinate of the best solution found so far */
  int *bestsolW;          /* the width of the best solution found so far */
  int *bestsolH;          /* the height of the best solution found so far */
  int *bestsolB;          /* the bin of the best solution found so far */
  /* Never modify the above four lines. */
  /* You can add more components below. */
}Vdata;     /* various data often necessary during the search */

/************************ declaration of functions ***************************/
void *malloc_e( size_t size );
void read_rectFile(char *filename,RectData *rectdata,Vdata *vdata);
void prepare_memory(Vdata *vdata,RectData *rectdata);
void recompute_obj(Vdata *vdata,RectData *rectdata);
void output_viwe_file(char *filename,Vdata *vdata,RectData *rectdata);

double compute_cost(Vdata *vdata,RectData *rectdata);
int is_feasible(Vdata *vdata,RectData *rectdata);
int check_overlap(int x1,int w1,int y1,int h1,int x2,int w2,int y2,int h2);


/***** malloc with error check ***********************************************/
void *malloc_e( size_t size ){
  void *s;
  if ( (s=malloc(size)) == NULL ) {
    fprintf( stderr, "malloc : not enough memory.\n" );
    exit(EXIT_FAILURE);
  }
  return s;
}

/***** open the file with rectangle *****************************************/
/***** NEVER MODIFY THIS SUBROUTINE! *****************************************/
void read_rectFile(char *filename,RectData *rectdata,Vdata *vdata){
  char buf[3][40];
  int id,height,width;
  FILE *fp;
  int itemCount = 0, curSize = 10,ret,j;
  
  fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("%s can not be found\n", filename);
    exit(1);
  }
  
  rectdata->id = (int *)malloc_e(curSize * sizeof(int));
  rectdata->height = (int *)malloc_e(curSize * sizeof(int));
  rectdata->width = (int *)malloc_e(curSize * sizeof(int));
  
  fscanf(fp, "%s;%s;%s", buf[0], buf[1], buf[2]);
  for (j = 0;; j++) {
    if ((ret = fscanf(fp, "%d;%d;%d", &id, &height, &width)) != EOF) {
      if (itemCount >= curSize) {
        curSize = curSize * 2;
        rectdata->id = (int *)realloc(rectdata->id, curSize * sizeof(int));
        rectdata->height = (int *)realloc(rectdata->height, curSize * sizeof(int));
        rectdata->width = (int *)realloc(rectdata->width, curSize * sizeof(int));
      }
      
      rectdata->id[j] = id;
      rectdata->height[j] = height;
      rectdata->width[j] = width;
      rectdata->areaofRects += height*width;
      itemCount++;
    }
    else break;
  }
  rectdata->NumberofRect = itemCount;
  //printf("NumberofRectangle:%d\n",rectdata->NumberofRect);
  prepare_memory(vdata,rectdata);
}

/***** prepare memory space **************************************************/
/***** Feel free to modify this subroutine. **********************************/
void prepare_memory(Vdata *vdata,RectData *rectdata){
  int n,i;
  n = rectdata->NumberofRect;
  vdata->bestsolId    = (int *)malloc_e(n * sizeof(int));
  vdata->bestsolX    = (int *)malloc_e(n * sizeof(int));
  vdata->bestsolY    = (int *)malloc_e(n * sizeof(int));
  vdata->bestsolW    = (int *)malloc_e(n * sizeof(int));
  vdata->bestsolH    = (int *)malloc_e(n * sizeof(int));
  vdata->bestsolB    = (int *)malloc_e(n * sizeof(int));
  /* the next line is just to give an initial solution */
  for(i = 0;i < n;i++){
    vdata->bestsolId[i] = i; vdata->bestsolB[i] = i;
    vdata->bestsolX[i] = 0; vdata->bestsolY[i] = 0;
    vdata->bestsolW[i] = rectdata->width[i]; vdata->bestsolH[i] = rectdata->height[i];
  }
  return;
}

/***** cost of the filling rate ******************************************************/
/***** NEVER MODIFY THIS SUBROUTINE! *****************************************/
double compute_cost(Vdata *vdata,RectData *rectdata){
  int i; double bin = -1,width = 0;
  for(i = 0;i < rectdata->NumberofRect;i++){
    if(bin < vdata->bestsolB[i])
      bin = vdata->bestsolB[i];
  }
  for(i = 0;i < rectdata->NumberofRect;i++){
    if(bin == vdata->bestsolB[i]){
      if(width < vdata->bestsolX[i] + vdata->bestsolW[i]){
        width = vdata->bestsolX[i] + vdata->bestsolW[i];
      }
    }
  }
  double cost = 0;
  cost = rectdata->areaofRects / (binArea * bin + binHeight * width) *100;
  return cost;
}

/***** check the feasibility of the placement *************************************/
/***** NEVER MODIFY THIS SUBROUTINE! *****************************************/
int is_feasible(Vdata *vdata,RectData *rectdata){
  int i,j,n = rectdata->NumberofRect,flag = 1;
  int *id = vdata->bestsolId;
  int *x = vdata->bestsolX;
  int *y = vdata->bestsolY;
  int *w = vdata->bestsolW;
  int *h = vdata->bestsolH;
  int *b = vdata->bestsolB;
  int *placed = (int *)malloc_e(n * sizeof(int));
  for(i = 0;i < n;i++)
    placed[i] = 0;
  
  for(i = 0;i < n;i++){
    placed[id[i]] = 1;
    if(b[i] < 0 || id[i] < 0){
      flag = 0; printf("bin id or rectangle id is not correct"); break;
    }
    if(x[i] + w[i] > binWidth || y[i] + h[i] > binHeight || x[i] < 0 || y[i] < 0){
      flag = 0; printf("the rectanle %d is not placed in bin\n",id[i]); break;
    }
    if(i != n - 1){
      for(j = i+1;j < n;j++){
        if(b[i] == b[j]){
          flag = check_overlap(x[i],w[i],y[i],h[i],x[j],w[j],y[j],h[j]);
          if(flag == 0)
            break;
        }
        else
          break;
      }
      if(flag == 0)
        break;
    }
  }
  if(flag == 1){
    for(i = 0;i < n;i++){
      if(placed[i] == 0){
        flag = 0; printf("the rectangle %d is not placed\n",placed[i]); break;
      }
      if(vdata->bestsolW[i] != rectdata->width[vdata->bestsolId[i]] || vdata->bestsolH[i] != rectdata->height[vdata->bestsolId[i]]){
        flag = 0; printf("bestsol data(width or height) of rectangle %d is not correct rectdata(width or height)\n",vdata->bestsolId[i]);
      }
    }
  }
  return flag;
}

/***** check the overlap rectangle *************************************/
/***** NEVER MODIFY THIS SUBROUTINE! *****************************************/
int check_overlap(int x1,int w1,int y1,int h1,int x2,int w2,int y2,int h2){
  int flag = 1;
  if(x1 + w1 > x2 && y1 + h1 > y2){
    if(x2 + w2 > x1 && y2 + h2 > y1){
      flag = 0; printf("rectangle overlap\n");
    }
  }
  return flag;
}

/***** check the feasibility and recompute the cost **************************/
/***** NEVER MODIFY THIS SUBROUTINE! *****************************************/
void recompute_obj(Vdata *vdata,RectData *rectdata){
  if(!is_feasible(vdata,rectdata)){
    printf("not feasible\n");
    exit(1);
  }
  int n = rectdata->NumberofRect;
  printf("number of used bin:%d \nfilling rate = %.2f%%\n",vdata->bestsolB[n - 1] + 1,compute_cost(vdata,rectdata));
  printf("time for the search:    %7.2f seconds\n",vdata->endtime - vdata->starttime);
  printf("time to read the instance: %7.2f seconds\n",vdata->starttime - vdata->timebird);
}

/***** output the tex file in the Place_VIEW format **********************************/
/***** NEVER MODIFY THIS SUBROUTINE! *****************************************/
void output_viwe_file(char *filename,Vdata *vdata,RectData *rectdata){
  FILE *fp;
  fp = fopen(filename,"w");
  if(fp == NULL){
    printf("%sファイルが開けません\n",filename);
    return;
  }
  fprintf(fp,"\\documentclass[a4paper]{article}\n");
  fprintf(fp,"\\usepackage{graphicx}\n");
  fprintf(fp,"\\usepackage{tikz}\n");
  fprintf(fp,"\\begin{document}\n");
  fprintf(fp,"\\scalebox{1.2}{\n");
  int i,n = rectdata->NumberofRect;
  int *id = vdata->bestsolId;
  int *x = vdata->bestsolX;
  int *y = vdata->bestsolY;
  int *w = vdata->bestsolW;
  int *h = vdata->bestsolH;
  int *b = vdata->bestsolB;
  double s = 0.002;
  for(i = 0;i < n;i++){
    if(i == 0){
      fprintf(fp,"\\begin{tikzpicture}\n");
      fprintf(fp,"\\draw (%d,%d) rectangle(%f,%f);\n",0,0,binWidth * s,binHeight * s);
      fprintf(fp,"\\draw[fill=cyan] (%f,%f) rectangle node{%d} (%f,%f);\n",x[i] * s,y[i] * s,id[i],x[i] * s + w[i] * s,y[i] * s + h[i] * s);
    }
    else if(i == n - 1){
      if(b[i - 1] != b[i]){
        fprintf(fp,"\\end{tikzpicture}\n");
        fprintf(fp,"}\n");
        fprintf(fp,"\\newline\\newline\n");
        fprintf(fp,"\\scalebox{1.2}{\n");
        fprintf(fp,"\\begin{tikzpicture}\n");
        fprintf(fp,"\\draw (%d,%d) rectangle(%f,%f);\n",0,0,binWidth * s,binHeight * s);
        fprintf(fp,"\\draw[fill=cyan] (%f,%f) rectangle node{%d} (%f,%f);\n",x[i] * s,y[i] * s,id[i],x[i] * s + w[i] * s,y[i] * s + h[i] * s);
      }
      else{
        fprintf(fp,"\\draw[fill=cyan] (%f,%f) rectangle node{%d} (%f,%f);\n",x[i] * s,y[i] * s,id[i],x[i] * s + w[i] * s,y[i] * s + h[i] * s);
      }
      fprintf(fp,"\\end{tikzpicture}\n");
      fprintf(fp,"}\n");
      fprintf(fp,"\\end{document}\n");
    }
    else if(b[i - 1] != b[i]){
      fprintf(fp,"\\end{tikzpicture}\n");
      fprintf(fp,"}\n");
      fprintf(fp,"\\newline\\newline\n");
      fprintf(fp,"\\scalebox{1.2}{\n");
      fprintf(fp,"\\begin{tikzpicture}\n");
      fprintf(fp,"\\draw (%d,%d) rectangle(%f,%f);\n",0,0,binWidth * s,binHeight * s);
      fprintf(fp,"\\draw[fill=cyan] (%f,%f) rectangle node{%d} (%f,%f);\n",x[i] * s,y[i] * s,id[i],x[i] * s + w[i] * s,y[i] * s + h[i] * s);
    }
    else{
      fprintf(fp,"\\draw[fill=cyan] (%f,%f) rectangle node{%d} (%f,%f);\n",x[i] * s,y[i] * s,id[i],x[i] * s + w[i] * s,y[i] * s + h[i] * s);
    }
  }
}

/***** main ******************************************************************/
int main(int argc,char *argv[]){
  RectData    rectdata;   /* data of rectangle instance */
  Vdata       vdata;      /* various data often needed during search */
  
  vdata.timebird = cpu_time();
  read_rectFile((char*)argv[1],&rectdata,&vdata);
  vdata.starttime = cpu_time();
  
  /*****
    
    Write your algorithm here.
    Of course you can add your subroutines outside main().
    At this point, the instance data is stored in the structure "rectdata".
    
    rectdata.id[k] :       the id of k th read rectangle
    rectdata.height[k] :   the height of k th read rectangle
    rectdata.width[k] :    the width of k th read rectangle
    rectdata.Numberofrect :    the number of rectangles n
    Never modify rectdata
    
    vdata.bestsolId[k] :    id of k th placed rectangle(k = 0,1,...,n-1)
    vdata.bestsolX[k] :     x-coordinate of k th placed rectangle(k = 0,1,...,n-1)
    vdata.bestsolY[k] :     y-coordinate of k th placed rectangle(k = 0,1,...,n-1)
    vdata.bestsolW[k] :     width of k th placed rectangle(k = 0,1,...,n-1)
    vdata.bestsolH[k] :     height of k th placed rectangle(k = 0,1,...,n-1)
    vdata.bestsolB[k] :     bin of k th rectangle placed(k = 0,1,...,n-1)
    
    Store your best solution in vdata.bestsol*, then "recompute_obj()"
    will compute its objective value and check its feasible.
    In this competition, rotation of rectangle is not allowed.
    
    *****/
  
  vdata.endtime = cpu_time();
  recompute_obj(&vdata,&rectdata);
  output_viwe_file((char*)argv[2],&vdata,&rectdata);
  return 0;
}
