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
    char *src = "481234781";
    char *ref = "4411327431";
    uint32_t maxLine = strlen(src) + 1;
    uint32_t maxColumn = strlen(ref) + 1;

    MatrixNode *matrix = new MatrixNode[maxLine*maxColumn];
    compareMatrix(matrix,(uint8_t*)src,(uint8_t*)ref,maxLine,maxColumn);
    dumpMatrixValue(matrix,maxLine,maxColumn);
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
	    if(src[i-1] == ref[j-1])
	    {
		(matrix + i * maxColumn + j) -> type = MATRIX_NODE_TYPE_KEY;
		
		(matrix + i * maxColumn + j) -> maxSubsequenceLen = 1 + (matrix + (i-1)*maxColumn + j-1) -> maxSubsequenceLen;

		if(MATRIX_NODE_TYPE_KEY == (matrix + (i-1)*maxColumn + j-1) -> type)
		{
		    // 记录上一相邻匹配节点，并增加相邻匹配节点权重
		    (matrix + i * maxColumn +j) -> preNode = (matrix + (i-1)*maxColumn + j-1);
		    (matrix + i * maxColumn +j) -> weight = (matrix + i * maxColumn +j) -> preNode->weight + 1;
		    MatrixNode *temp = (matrix + i * maxColumn +j) -> preNode;
		    do{
			temp->weight ++;
			temp = temp -> preNode;
		    }
		    while(NULL != temp);		    
		}else{
		    (matrix + i * maxColumn +j) -> weight = 1; // 第一个相邻匹配节点，权重为1
		}
	    }else{
		(matrix + i * maxColumn + j) -> maxSubsequenceLen =
		    ((matrix + i*maxColumn + j-1) -> maxSubsequenceLen > (matrix + (i-1)*maxColumn + j) ->  maxSubsequenceLen)?
		    (matrix + i*maxColumn + j-1) -> maxSubsequenceLen:
		    (matrix + (i-1)*maxColumn + j) -> maxSubsequenceLen;
	    }
	}
    }
}
void TxtComparator::outputMatrix(MatrixNode *matrix,uint64_t maxLine,uint64_t maxColumn,const uint8_t *outputFile)
{
    // 
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

    printf("\r\n");

    for(i = 0; i < maxLine; i++)
    {
	for(j = 0; j < maxColumn; j++)
	{
	    printf("%2d",(matrix + i * maxColumn +j) -> weight);
	}
	printf("\r\n");
    }

}



















