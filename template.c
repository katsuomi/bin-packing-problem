#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <math.h>
#include "cpu_time.c"

#define binWidth    6000.0  /* the width of bin */
#define binHeight   3210.0  /* the height of bin */
#define binArea     binWidth*binHeight  /* the area of bin */
#define TIMELIM     300 /* the time limit for the algorithm in seconds */
#define SIZE_OF_ARRAY(array)    (sizeof(array)/sizeof(array[0]))

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
        //else
          //break;
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
  int NumberofBin = 0;;
  for(int i = 0;i < n;i++){
    if(NumberofBin < vdata->bestsolB[i])
      NumberofBin = vdata->bestsolB[i];
  }
  printf("number of used bin:%d \nfilling rate = %.2f%%\n",NumberofBin + 1,compute_cost(vdata,rectdata));
  printf("time for the search:    %7.2f seconds\n",vdata->endtime - vdata->starttime);
  printf("time to read the instance: %7.2f seconds\n",vdata->starttime - vdata->timebird);
  printf("elapsed time: %fs\n",cpu_time() - vdata->starttime);
}

/***** output the tex file in the Place_VIEW format **********************************/
/***** NEVER MODIFY THIS SUBROUTINE! *****************************************/
void output_viwe_file(char *filename,Vdata *vdata,RectData *rectdata){
  FILE *fp;
  fp = fopen(filename,"w");
  if(fp == NULL){
    printf("file is not opened: %s\n",filename);
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
    
  int j,temp=0;
  for(i = 0;i < n;i++){
    for(j = n-1;j>i;j--){
      if(b[j-1]>b[j]){
        temp = id[j-1]; id[j-1] = id[j]; id[j] = temp;
        temp = x[j-1]; x[j-1] = x[j]; x[j] = temp;
        temp = y[j-1]; y[j-1] = y[j]; y[j] = temp;
        temp = w[j-1]; w[j-1] = w[j]; w[j] = temp;
        temp = h[j-1]; h[j-1] = h[j]; h[j] = temp;
        temp = b[j-1]; b[j-1] = b[j]; b[j] = temp;
      }
    }
  }
    
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

int compare_int(const void *a, const void *b){
  return *(int*)b - *(int*)a;
}

size_t array_count_values(const int* array, size_t size, int value){
  size_t count = 0;
  for (size_t i = 0; i < size; ++i){
    if (array[i] == value) {
      ++count;
    }
  }
  return count;
}

int sum,i,j = 0;
int indexToWidthOfCol(int index, int n, int *NumRectInCol, int *WidthOfCol){
	for(i = j;i < n;i++){
    sum += NumRectInCol[i];
    if(sum >= index){
      j = i + 1;
      return WidthOfCol[i];
      break;
    }
  }
  return 0;
}

int indexOfColToBestSol(int index, int *bestsol, int *NumRectInCol,int count){
  sum = 0;
	for(i = 0;i < index;i++){
    sum += NumRectInCol[i];
  }
  return bestsol[sum+count];
}

int binIdToLastX(int binId,int n, int *bestsolB, int *bestsolX){
  int x = -99;
	for(i = 0;i < n;i++){
    if(binId == bestsolB[i]){
      if(bestsolX[i] > x){
        x = bestsolX[i];
      }
    }
  }
  return x;
}

int binIdToLastWidth(int binId,int n, int *bestsolB, int *bestsolX,int *bestsolW){
  int x = -99;
  int width = -99;
	for(i = 0;i < n;i++){
    if(binId == bestsolB[i]){
      if(bestsolX[i] > x){
        x = bestsolX[i];
        width = bestsolW[i];
      }
    }
  }
  return width;
}

int xToMaxWidth(int binId,int x,int n,int *bestsolB, int *bestsolX,int *bestsolW){
  for(j = 0;j < n;j++){
    if(bestsolB[j] == binId && bestsolX[j] == x){
      return bestsolW[j];
    }
  }
  return 0;
}

int xToItemCounts(int binId,int x,int n,int *bestsolB, int *bestsolX){
  int count = 0;
  for(j = 0;j < n;j++){
    if(bestsolB[j] == binId && bestsolX[j] == x){
      count++;
    }
  }
  return count;
}

int xToColHeight(int binId,int x,int n,int *bestsolB, int *bestsolX,int *bestsolY, int *bestsolH){
  int tmpHeight = -99;

  for(j = 0;j < n;j++){
    if(bestsolB[j] == binId && bestsolX[j] == x){
      if(bestsolY[j] + bestsolH[j] > tmpHeight){
        tmpHeight = bestsolY[j] + bestsolH[j];
      }
    }
  }

  return tmpHeight;
}

int binIdToIsManyCols(int binId,int n,int *bestsolB, int *bestsolX){
  int count = 0;
  int prevX = -99;

  for(j = 0;j < n;j++){
    if(bestsolB[j] == binId && prevX != bestsolX[j]){
      prevX = bestsolX[j];
      count++;
    }
  }

  if(count == 1){
    return 0;
  }else{
    return 1;
  }
}

void my_algorithm(RectData *rectdata, Vdata *vdata){
	int n = rectdata->NumberofRect;
	int *NumRectInCol = (int *)malloc_e(n * sizeof(int));
	int *WidthOfCol = (int *)malloc_e(n * sizeof(int));
	int i,k,l,m,p,h,w,totalHeight,tmp = 0;
	int columnPos = 0;
	int width = 0;
	int count = 0;
  int count2 = 0;
  int count3 = 0;
  bool isTrue = true;
  bool isOk = false;
  int *dataWidth = (int *)malloc_e(n * sizeof(int));
  int *dataWidthIndex = (int *)malloc_e(n * sizeof(int));
  int *binIds = (int *)malloc_e(n * sizeof(int));
  int *selectedWidth = (int *)malloc_e(n * sizeof(int));
  int *selectedX = (int *)malloc_e(n * sizeof(int));
  int *selectedXSorted = (int *)malloc_e(n * sizeof(int));
  int uniqIndex[n];
  int uniqIndex2[n];

	for(i = 0;i < n;i++){
		NumRectInCol[i] = -99; 
		WidthOfCol[i] = -99;
    dataWidthIndex[i] = 0;
    uniqIndex[i] = 0; 
    uniqIndex2[i] = 0;    
    binIds[i] = -99;
    selectedWidth[i] = -99;
    selectedX[i] = -99;
    selectedXSorted[i] = -99;
	}

	for(i = 0;i < n;i++){
    dataWidth[i] = rectdata->width[i];
  }

  qsort(dataWidth, n, sizeof(int), compare_int);

  for(i = 0;i < n;i++){
    for(k = 0;k < n;k++){
      if(dataWidth[i] == rectdata->width[k] && !(array_count_values(uniqIndex, SIZE_OF_ARRAY(uniqIndex), k))){
        dataWidthIndex[i] = k;
        uniqIndex[count] = k;
        count++;
        break;
      }
    }
  }

	count = 0;
	// ビンの高さを超えない範囲で、アイテムを積み重ねていく/超えてしまった場合、横に並べる
  for(i = 0;i < n;i++){
    h = rectdata->height[dataWidthIndex[i]]; 
    w = rectdata->width[dataWidthIndex[i]];
    totalHeight = totalHeight + h;
    if(totalHeight > binHeight){
      totalHeight = h;
      if(i == n - 1){
        WidthOfCol[columnPos] = width; width = w;
        NumRectInCol[columnPos] = count; columnPos++;
        WidthOfCol[columnPos] = width; NumRectInCol[columnPos] = 1; columnPos++;
      }
      else{
        WidthOfCol[columnPos] = width; width = w;
        NumRectInCol[columnPos] = count; columnPos++; count = 1;
      }
    }
    else{
      if(width < w)
        width = w;
      count++;
      if(i == n - 1){
        WidthOfCol[columnPos] = width; NumRectInCol[columnPos] = count; columnPos++;
      }
    }
  }

  // アイテムがどのビンに属するのかを決定/情報をvdataへ格納
	int b = 0, x = 0, y = 0, j = 0; count = 0;
	for(i = 0;i < columnPos;i++){
    if(x + WidthOfCol[i] > binWidth){
  		x = 0; y = 0; b++;
		}
		for(j = 0;j < NumRectInCol[i];j++){
			vdata->bestsolId[count] = rectdata->id[dataWidthIndex[count]]; vdata->bestsolW[count] = rectdata->width[dataWidthIndex[count]]; vdata->bestsolH[count] = rectdata->height[dataWidthIndex[count]]; vdata->bestsolB[count] = b; vdata->bestsolX[count] = x; vdata->bestsolY[count] = y;
			y = y + rectdata->height[dataWidthIndex[count]]; 
			count++;
		}
		x = x + WidthOfCol[i]; 
		y = 0;
	}

  int prevBinId = 0;
  int binCounts = vdata->bestsolB[n - 1] + 1;
  int lastEmptyWidthArray[binCounts];

  /***** 高さに対するアプローチここから ******************************************************************/

  int *emptyHeights = (int *)malloc_e(columnPos * sizeof(int));
  int *emptyHeightsBinIds = (int *)malloc_e(columnPos * sizeof(int));
  int *emptyHeightsX = (int *)malloc_e(columnPos * sizeof(int));
  int *emptyHeightsMaxW = (int *)malloc_e(columnPos * sizeof(int));
  int *tmpWs = (int *)malloc_e(n * sizeof(int));
  int *tmpXsSorted = (int *)malloc_e(n * sizeof(int));
  int *tmpEmptyHeightsBinIds = (int *)malloc_e(n * sizeof(int));
  int *tmpEmptyHeightsX = (int *)malloc_e(n * sizeof(int));
  int *tmpEmptyHeights = (int *)malloc_e(n * sizeof(int));
  int *tmpBestsolId = (int *)malloc_e(n * sizeof(int));
  int *tmpBestsolB = (int *)malloc_e(n * sizeof(int));
  int *tmpBestsolX = (int *)malloc_e(n * sizeof(int));
  int tmpBinIds[n];
  int tmpXs[n];
  int count5 = 0;

  while(count5 < 20){
    for(i = 0;i < columnPos;i++){
      emptyHeights[i] = -99;
      emptyHeightsBinIds[i] = -99;
      emptyHeightsX[i] = -99;
    }

    for(i = 0;i < n;i++){
      tmpBinIds[i] = -99;
      tmpXs[i] = -99;
      tmpWs[i] = -99;
      tmpXsSorted[i]= -99;
      uniqIndex[i] = -99;
      uniqIndex2[i] = -99;
      tmpEmptyHeightsBinIds[i] = -99;
      tmpEmptyHeightsX[i] = -99;
      tmpEmptyHeights[i] = -99;
      tmpBestsolId[i] = -99;
      tmpBestsolB[i] = -99;
      tmpBestsolX[i] = -99;
    }

    int prevX = -99;
    int tmpEmptyHeight = -99;  
    int tmpEmptyHeightBinId = -99;
    int tmpEmptyHeightX = -99;
    count = 0;
    for(i = 0;i < binCounts;i++){
      isTrue = false;
      for(j = 0;j < n;j++){
        if(vdata->bestsolB[j] == i){
          if(binIdToIsManyCols(vdata->bestsolB[j],n,vdata->bestsolB,vdata->bestsolX) == 1){
            if(prevX != vdata->bestsolX[j] && (!(array_count_values(tmpBinIds, SIZE_OF_ARRAY(tmpBinIds), vdata->bestsolB[j])) && !(array_count_values(tmpXs, SIZE_OF_ARRAY(tmpXs), vdata->bestsolX[j])))){
              tmpBinIds[count] = vdata->bestsolB[j];
              tmpXs[count] = vdata->bestsolX[j];
              emptyHeights[count] = 3210 - xToColHeight(vdata->bestsolB[j],vdata->bestsolX[j],n,vdata->bestsolB,vdata->bestsolX,vdata->bestsolY,vdata->bestsolH);
              emptyHeightsBinIds[count] = vdata->bestsolB[j];
              emptyHeightsX[count] = vdata->bestsolX[j];
              count++;
            }
            prevX = vdata->bestsolX[j];
          }else{
            tmpBinIds[count] = vdata->bestsolB[j];
            tmpXs[count] = vdata->bestsolX[j];
            emptyHeights[count] = 3210 - xToColHeight(vdata->bestsolB[j],vdata->bestsolX[j],n,vdata->bestsolB,vdata->bestsolX,vdata->bestsolY,vdata->bestsolH);
            emptyHeightsBinIds[count] = vdata->bestsolB[j];
            emptyHeightsX[count] = vdata->bestsolX[j];
            isTrue = true;
          }
        }
      }
      if(isTrue){
        count++;
      }
    }

    for(i = 0;i < n;i++){
      tmpBinIds[i] = -99;
      tmpXs[i] = -99;
    }

    int tmpX = -99;
    count2 = 0;
    count3 = 0;
    for(i = 0;i < binCounts;i++){
      for(j = 0;j < n;j++){
        int count4 = 0;
        if(vdata->bestsolB[j] == i && vdata->bestsolX[j] != tmpX){
          tmpX = vdata->bestsolX[j];
          int tmpCount = count3;
          for(k = 0;k < n;k++){
            if(tmpX == vdata->bestsolX[k] && i == vdata->bestsolB[k]){
              for(l = 0;l < columnPos;l++){
                if(!(array_count_values(tmpBinIds, SIZE_OF_ARRAY(tmpBinIds), emptyHeightsBinIds[l])) && !(array_count_values(tmpXs, SIZE_OF_ARRAY(tmpXs), emptyHeightsX[l]))  && !(emptyHeightsBinIds[l] == vdata->bestsolB[k] && emptyHeightsX[l] == vdata->bestsolX[k]) && emptyHeightsBinIds[l] != -99 && !(array_count_values(uniqIndex, SIZE_OF_ARRAY(uniqIndex), l)) && !(array_count_values(uniqIndex2, SIZE_OF_ARRAY(uniqIndex2), k)) && vdata->bestsolW[k] <= xToMaxWidth(emptyHeightsBinIds[l],emptyHeightsX[l],n,vdata->bestsolB,vdata->bestsolX,vdata->bestsolW) && vdata->bestsolH[k] <= emptyHeights[l]){
                  tmpEmptyHeightsBinIds[count3] = emptyHeightsBinIds[l];
                  tmpEmptyHeightsX[count3] = emptyHeightsX[l];
                  tmpEmptyHeights[count3] = emptyHeights[l];
                  tmpBestsolId[count3] = vdata->bestsolId[k];
                  tmpBestsolB[count3] = vdata->bestsolB[k];
                  tmpBestsolX[count3] = vdata->bestsolX[k];
                  uniqIndex[count3] = l;
                  uniqIndex2[count3] = k;
                  count3++;
                  count4++;
                  break;
                }
              }
            }
          }
          if(count4 == xToItemCounts(vdata->bestsolB[j],vdata->bestsolX[j],n,vdata->bestsolB,vdata->bestsolX)){
            tmpBinIds[count2] = vdata->bestsolB[j];
            tmpXs[count2] = vdata->bestsolX[j];
            tmpWs[count2] = xToMaxWidth(vdata->bestsolB[j],vdata->bestsolX[j],n,vdata->bestsolB,vdata->bestsolX,vdata->bestsolW);
            count2++; 
          }else{
            for(m = tmpCount;m < count3;m++){
              uniqIndex[m] = -99;
            }
          }
        }
        if(tmpX == 0){
          tmpX = -99;
        }
      }
    }

    for(i = 0;i < n;i++){
      uniqIndex[i] = -99;
    }

    int tmpBinId = -99;
    tmpX = -99;
    count2 = 0;
    for(i = 0;i < n;i++){
      if(tmpBinIds[i] != -99){
        for(j = 0;j < n;j++){
          if(tmpBestsolB[j] == tmpBinIds[i] && tmpBestsolX[j] == tmpXs[i]){
            for(k = 0;k < n;k++){
              if(tmpBestsolId[j] == vdata->bestsolId[k]){
                vdata->bestsolX[k] = tmpEmptyHeightsX[j];
                vdata->bestsolB[k] = tmpEmptyHeightsBinIds[j];
                vdata->bestsolY[k] = 3210 - tmpEmptyHeights[j];
              }
            }
          }
        }
      }
    }

    // 左にずらす処理
    for(i = 0;i < n;i++){
      tmpXsSorted[i] = tmpXs[i];
    }

    qsort(tmpXsSorted, n, sizeof(int), compare_int);

    for(i = 0;i < n;i++){
      if(tmpXsSorted[i] != -99){
        for(j = 0;j < n;j++){
          if(tmpBinIds[j] != -99 && tmpXsSorted[i] == tmpXs[j]){
            for(k = 0;k < n;k++){
              if(tmpBinIds[j] == vdata->bestsolB[k] && vdata->bestsolX[k] > tmpXs[j]){
                vdata->bestsolX[k] = vdata->bestsolX[k] - tmpWs[j];
              }
            }
            tmpXs[j] = -99;
          }
        }
      }
    }

    count5++;
  }

  /***** 高さに対するアプローチここまで ******************************************************************/

  /***** 幅に対するアプローチ ******************************************************************/

  // 以下、空いたスペースにアイテムを積む、左に寄せるを繰り返す
  count2 = 0;
  isTrue = true;
  while (isTrue){
    for(i = 0;i < binCounts;i++){
      lastEmptyWidthArray[i] = -99;
    }

    int prevX = -99;

    for(i = 0;i < binCounts;i++){
      if(binIdToLastX(i,n,vdata->bestsolB,vdata->bestsolX) == -99){
        lastEmptyWidthArray[i] = 0;
      }else{
        lastEmptyWidthArray[i] = 6000 - (binIdToLastX(i,n,vdata->bestsolB,vdata->bestsolX) + binIdToLastWidth(i,n,vdata->bestsolB,vdata->bestsolX,vdata->bestsolW));
      }
    }

    // 詰める処理
    for(i = 0;i < n;i++){ 
      binIds[i] = -99;
      selectedWidth[i] = -99;
      selectedX[i] = -99;
      selectedXSorted[i] = -99;
    }

    prevX = -99;
    count = 0;
    for(i = 0;i < binCounts;i++){
      if(binIdToLastX(i,n,vdata->bestsolB,vdata->bestsolX) != -99){
        for(j = 0;j < n;j++){
          if(prevX != -99 && prevX != vdata->bestsolX[j] && i < vdata->bestsolB[j] && lastEmptyWidthArray[i] >= xToMaxWidth(vdata->bestsolB[j],vdata->bestsolX[j],n,vdata->bestsolB,vdata->bestsolX,vdata->bestsolW)){
            int binId = vdata->bestsolB[j];
            int x = vdata->bestsolX[j];
            binIds[count] = binId;
            selectedWidth[count] = vdata->bestsolW[j];
            selectedX[count] = x;
            count++;

            for(k = j;k < n;k++){
              if(vdata->bestsolB[k] == binId && vdata->bestsolX[k] == x){
                vdata->bestsolB[k] = i;
                vdata->bestsolX[k] = 6000 - lastEmptyWidthArray[i];
              }
            }
            prevX = -99;
            break;
          }
          prevX = vdata->bestsolX[j];
        }
      }
    }

    if(count == 0){
      isTrue = false;
    }

    // 左にずらす処理

    for(i = 0;i < n;i++){
      selectedXSorted[i] = selectedX[i];
    }

    qsort(selectedXSorted, n, sizeof(int), compare_int);

    for(i = 0;i < n;i++){
      if(selectedXSorted[i] != -99){
        for(j = 0;j < n;j++){
        if(binIds[j] != -99 && selectedXSorted[i] == selectedX[j]){
          for(k = 0;k < n;k++){
            if(binIds[j] == vdata->bestsolB[k] && vdata->bestsolX[k] > selectedX[j]){
              vdata->bestsolX[k] = vdata->bestsolX[k] - selectedWidth[j];
            }
          }
          selectedX[j] = -99;
        }
        }
      }
    }
    count2++;
  }

  /***** 幅に対するアプローチここまで ******************************************************************/


  /***** 工夫ここから ******************************************************************/
    // int maxHeightOfCols[columnPos];
    // int totalWidthOfCols[columnPos];
    // int binIdOfCols[columnPos];
    // int xOfCols[columnPos];

    // for(i = 0;i < n;i++){
    //   tmpBinIds[i] = -99;
    //   tmpXs[i] = -99;
    // }

    // for(i = 0;i < columnPos;i++){
    //   emptyHeights[i] = -99;
    //   emptyHeightsBinIds[i] = -99;
    //   emptyHeightsX[i] = -99;
    //   emptyHeightsMaxW[i] = -99;
    //   maxHeightOfCols[i] = -99;
    //   totalWidthOfCols[i] = -99;
    //   binIdOfCols[i] = -99;
    //   xOfCols[i] = -99;
    // }

    // int prevX = -99;
    // count = 0;
    // for(i = 0;i < binCounts;i++){
    //   isTrue = false;
    //   for(j = 0;j < n;j++){
    //     if(vdata->bestsolB[j] == i){
    //       if(binIdToIsManyCols(vdata->bestsolB[j],n,vdata->bestsolB,vdata->bestsolX) == 1){
    //         if(prevX != vdata->bestsolX[j] && (!(array_count_values(tmpBinIds, SIZE_OF_ARRAY(tmpBinIds), vdata->bestsolB[j])) && !(array_count_values(tmpXs, SIZE_OF_ARRAY(tmpXs), vdata->bestsolX[j])))){
    //           tmpBinIds[count] = vdata->bestsolB[j];
    //           tmpXs[count] = vdata->bestsolX[j];
    //           emptyHeights[count] = 3210 - xToColHeight(vdata->bestsolB[j],vdata->bestsolX[j],n,vdata->bestsolB,vdata->bestsolX,vdata->bestsolY,vdata->bestsolH);
    //           emptyHeightsBinIds[count] = vdata->bestsolB[j];
    //           emptyHeightsX[count] = vdata->bestsolX[j];
    //           emptyHeightsMaxW[count] = xToMaxWidth(vdata->bestsolB[j],vdata->bestsolX[j],n,vdata->bestsolB,vdata->bestsolX,vdata->bestsolW);
    //           count++;
    //         }
    //         prevX = vdata->bestsolX[j];
    //       }else{
    //         tmpBinIds[count] = vdata->bestsolB[j];
    //         tmpXs[count] = vdata->bestsolX[j];
    //         emptyHeights[count] = 3210 - xToColHeight(vdata->bestsolB[j],vdata->bestsolX[j],n,vdata->bestsolB,vdata->bestsolX,vdata->bestsolY,vdata->bestsolH);
    //         emptyHeightsBinIds[count] = vdata->bestsolB[j];
    //         emptyHeightsX[count] = vdata->bestsolX[j];
    //         emptyHeightsMaxW[count] = xToMaxWidth(vdata->bestsolB[j],vdata->bestsolX[j],n,vdata->bestsolB,vdata->bestsolX,vdata->bestsolW);
    //         isTrue = true;
    //       }
    //     }
    //   }
    //   if(isTrue){
    //     count++;
    //   }
    // }

    // count = 0;
    // for(i = 0;i < n;i++){
    //   int tmpSumOfW = 0;
    //   int tmpH = 0;
    //   if((!(array_count_values(tmpBinIds, SIZE_OF_ARRAY(tmpBinIds), vdata->bestsolB[i])) && !(array_count_values(tmpXs, SIZE_OF_ARRAY(tmpXs), vdata->bestsolX[i])))){
    //     for(j = 0;j < n;j++){
    //       if(vdata->bestsolB[i] == vdata->bestsolB[j] && vdata->bestsolX[i] == vdata->bestsolX[j]){
    //         tmpSumOfW += vdata->bestsolW[j];
    //         if(vdata->bestsolH[j] > tmpH){
    //           tmpH = vdata->bestsolH[j];
    //         }
    //       }
    //     }
    //     maxHeightOfCols[count] = tmpH;
    //     totalWidthOfCols[count] = tmpSumOfW;
    //     binIdOfCols[count] = vdata->bestsolB[i];
    //     xOfCols[count] = vdata->bestsolX[i];
    //     tmpBinIds[i] = vdata->bestsolB[i];
    //     tmpXs[i] = vdata->bestsolX[i];
    //   }
    // }

  /***** 工夫ここまで ******************************************************************/

  // for(i = 0;i < binCounts;i++){
  //   printf("[i]: %d\n",i);
  //   printf("lastEmptyWidthArray: %d\n",lastEmptyWidthArray[i]);
  //   printf("binIdToLastX(i,n,vdata->bestsolB,vdata->bestsolX): %d\n",binIdToLastX(i,n,vdata->bestsolB,vdata->bestsolX));
  //   printf("binIdToLastWidth(i,n,vdata->bestsolB,vdata->bestsolX,vdata->bestsolW): %d\n",binIdToLastWidth(i,n,vdata->bestsolB,vdata->bestsolX,vdata->bestsolW));
  // }

  // for(i = 0;i < columnPos;i++){
  //   // printf("emptyHeightsBinIds: %d\n",emptyHeightsBinIds[i]);
  //   // printf("emptyHeights: %d\n",emptyHeights[i]);
  //   // printf("emptyHeightsX: %d\n",emptyHeightsX[i]);
  //   // printf("xToColHeight: %d\n",xToColHeight(0,0,n,vdata->bestsolB,vdata->bestsolX,vdata->bestsolY,vdata->bestsolH));
  // }

  // for(i = 0;i < n;i++){
  // //   // printf("bestsolB: %d\n",vdata->bestsolB[i]);
  // //   // printf("bestsolId: %d\n",vdata->bestsolId[i]);
  // //   // printf("bestsolW: %d\n",vdata->bestsolW[i]);
  // //   // printf("bestsolH: %d\n",vdata->bestsolH[i]);
  // //   // printf("bestsolX: %d\n",vdata->bestsolX[i]);
  // //   // printf("bestsolY: %d\n",vdata->bestsolY[i]);
  // //   // // printf("binCounts: %d\n",binCounts);
  //   // printf("tmpBinIds: %d\n",tmpBinIds[i]);
  //   // printf("tmpXs: %d\n",tmpXs[i]);
  // //   // printf("xToItemCounts: %d\n",xToItemCounts(8,1889,n,vdata->bestsolB,vdata->bestsolX));
  // }

	free(NumRectInCol);free(WidthOfCol);free(dataWidth);free(dataWidthIndex);free(binIds);free(selectedWidth);free(selectedX);free(selectedXSorted);free(emptyHeights);
  free(emptyHeightsBinIds);free(emptyHeightsX);free(emptyHeightsMaxW);free(tmpWs);free(tmpXsSorted);free(tmpEmptyHeightsBinIds);free(tmpEmptyHeightsX);free(tmpEmptyHeights);free(tmpBestsolId);
  free(tmpBestsolB);free(tmpBestsolX);
}

int main(int argc,char *argv[]){
  RectData    rectdata;   /* data of rectangle instance */
  Vdata       vdata;      /* various data often needed during search */
  
  vdata.timebird = cpu_time();
  read_rectFile((char*)argv[1],&rectdata,&vdata);
  vdata.starttime = cpu_time();
  
	my_algorithm(&rectdata,&vdata);
  
  vdata.endtime = cpu_time();
  recompute_obj(&vdata,&rectdata);
  output_viwe_file((char*)argv[2],&vdata,&rectdata);
  return 0;
}
