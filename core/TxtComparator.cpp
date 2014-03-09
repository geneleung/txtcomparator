#include "stddefs.h"
#include <string.h>
#include "core/TxtComparator.h"

#define LOG_TAG "TXT_COMPARATOR"

TxtComparator::TxtComparator()
{
    // 
}
TxtComparator::~TxtComparator()
{
    //
    LOGD("destroy TxtComparator!");
}
void TxtComparator::compare(const uint8_t *srcFile,const uint8_t *refFile)
{
    // char *src = "481234781";
    // char *ref = "4411327431";
    char *src = "2325167";
    char *ref = "1233257";
    // char *src = "1111223336668800000";
    // char *ref = "22335555555666678890000001";
    // char *src = "481234781";
    // char *ref = "4411327431";


    uint32_t maxLine = strlen(src) + 1;
    uint32_t maxColumn = strlen(ref) + 1;

    MatrixNode *matrix = new MatrixNode[maxLine*maxColumn];
    compareMatrix(matrix,(uint8_t*)src,(uint8_t*)ref,maxLine,maxColumn);
    dumpMatrixValue(matrix,maxLine,maxColumn);
    // outputMatrix(matrix,maxLine,maxColumn,NULL);
}
void TxtComparator::compareMatrix(MatrixNode *matrix,const uint8_t *src,const uint8_t *ref,uint64_t maxLine,uint64_t maxColumn)
{
    int i,j;
    i = j = 0;
    // step 1. init matrix,matrix size [srcLen + 1][refLen + 1]
    memset(matrix,0,sizeof(MatrixNode) * maxLine * maxColumn);

    // step 2.
    // 对比矩阵第0行和第0列初始化为0，他们不属于src or ref，他们的存在只是为了算法的完整性，使得计算简洁，所以循环我们从第1行和第1列开始
    for(i = 1; i < maxLine; i++)
    {
	for(j = 1; j< maxColumn; j++)
	{
	    (matrix + i * maxColumn + j) -> value = src[i-1]; // :无论是add,del or modify，所有节点的值都只需要用到src的值
	    
	    if(src[i-1] == ref[j-1])
	    {
		(matrix + i * maxColumn + j) -> type = MATRIX_NODE_TYPE_KEY;
		
		(matrix + i * maxColumn + j) -> maxSubsequenceLen = 1 + (matrix + (i-1)*maxColumn + j-1) -> maxSubsequenceLen;
		
	    }else{
		(matrix + i * maxColumn + j) -> maxSubsequenceLen =
		    ((matrix + i*maxColumn + j-1) -> maxSubsequenceLen > (matrix + (i-1)*maxColumn + j) ->  maxSubsequenceLen)?
		    (matrix + i*maxColumn + j-1) -> maxSubsequenceLen:
		    (matrix + (i-1)*maxColumn + j) -> maxSubsequenceLen;
	    }
	}
    }
}
/*
 * 1 a max
 * 2 b max
 * 3 c max
 * 4 b == c max
 * 5 a == c max
 * 6 a == b max
 * 7 a == b == c
 */
static int getMaxLocation(uint64_t a,uint64_t b,uint64_t c)
{
    if(a == b)
    {
	if(a == c)
	    return 7;
	if(a > c)
	    return 6;
    }
    if(a == c)
	if(a > b)
	    return 5;
    if(b == c)
	if(b > a)
	    return 4;
    return a>b?(a>c?1:3):(b>c?2:3);  
}
static void moveDiag(TxtComparator::MatrixNode * matrix,uint64_t *pi,uint64_t *pj)
{
    uint64_t i = *pi;
    uint64_t j = *pj;
    
    matrix->get(i-1,j-1) ->  nextNode = matrix->get(i,j);
    // 节点类型由外部修改 key or modify
    *pi = i - 1;
    *pj = j - 1;
}
static void moveUp(TxtComparator::MatrixNode * matrix,uint64_t *pi,uint64_t *pj)
{
    uint64_t i = *pi;
    uint64_t j = *pj;
    matrix->get(i-1,j) -> nextNode = matrix->get(i,j);
    matrix->get(i,j) -> type = TxtComparator::MATRIX_NODE_TYPE_ADD;
    *pi = i - 1;
}
static void moveLeft(TxtComparator::MatrixNode * matrix,uint64_t *pi,uint64_t *pj)
{
    uint64_t i = *pi;
    uint64_t j = *pj;
    matrix->get(i,j-1) -> nextNode = matrix->get(i,j);
    matrix->get(i,j) -> type = TxtComparator::MATRIX_NODE_TYPE_DEL;
    *pj = j - 1;
}
void TxtComparator::outputMatrix(MatrixNode *matrix,uint64_t maxLine,uint64_t maxColumn,const uint8_t *outputFile)
{
    //
    uint64_t i = maxLine -1;
    uint64_t j = maxColumn -1;
    do{
	if((i > 0) && (j > 0))
	{
	    int max = getMaxLocation((matrix + (i-1) * maxColumn + j-1) -> maxSubsequenceLen,(matrix + (i-1) * maxColumn + j) -> maxSubsequenceLen,(matrix + i * maxColumn + j-1) -> maxSubsequenceLen);
	    if(MATRIX_NODE_TYPE_KEY == (matrix + i * maxColumn + j) -> type)
	    {
		switch(max)
		{
		case 1:
		case 5:
		case 6:
		case 7:
		    // matrix->get(i,j)->type = MATRIX_NODE_TYPE_KEY;
		    moveDiag(matrix,&i,&j);break;
		case 2:
		    moveUp(matrix,&i,&j);break;
		case 3:
		    moveLeft(matrix,&i,&j);break;
		case 4:
		    if(i < j)
			moveUp(matrix,&i,&j);
		    else
			moveLeft(matrix,&i,&j);
		    break;
		default:
		    // do nothing
		    LOGE("never come here");
		}	
	    }else{
		switch(max)
		{
		case 1:
		    matrix->get(i,j)->type = MATRIX_NODE_TYPE_MODIFY;
		    moveDiag(matrix,&i,&j);break;
		case 2:
		    moveUp(matrix,&i,&j);break;
		case 3:
		    moveLeft(matrix,&i,&j);break;
		case 4:
		    if(i < j)
			moveUp(matrix,&i,&j);
		    else
			moveLeft(matrix,&i,&j);
		    break;
		case 5:
		    if(i > j)
			moveLeft(matrix,&i,&j);
		    else{
			matrix->get(i,j)->type = MATRIX_NODE_TYPE_MODIFY;
			moveDiag(matrix,&i,&j);
		    }
		    break;
		case 6:
		    if(i < j)
			moveUp(matrix,&i,&j);
		    else{
			matrix->get(i,j)->type = MATRIX_NODE_TYPE_MODIFY;
			moveDiag(matrix,&i,&j);
		    }
		    break;
		case 7:
		    if(i > j)
			moveLeft(matrix,&i,&j);
		    else if(i < j)
			moveUp(matrix,&i,&j);
		    else{
			matrix->get(i,j)->type = MATRIX_NODE_TYPE_MODIFY;
			moveDiag(matrix,&i,&j);
		    }
		default:
		    // do nothing
		    LOGE("never come here");
		}
	    }
	}else if(i > 0){
	    
	}else if(j > 0){
	    
	}else{
	    break;
	}
    }while(true);
}
TxtComparator::MatrixNode *TxtComparator::MatrixNode::get(uint64_t i,uint64_t j)
{
    return this + i * maxColumn + j;
}
void TxtComparator::dumpMatrixValue(MatrixNode *matrix,uint64_t maxLine,uint64_t maxColumn)
{
    int i,j;
    for(i = 0; i < maxLine; i++)
    {
	for(j = 0; j < maxColumn; j++)
	{
	    printf("%2d",(matrix + i * maxColumn +j) -> maxSubsequenceLen);
	}
	printf("\r\n");
    }
}



















