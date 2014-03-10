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
void TxtComparator::compare(const char *srcFile,const char *refFile)
{
    // char *src = "481234781";
    // char *ref = "4411327431";
    // char *src = "2325167";
    // char *ref = "1233257";
    // char *src = "1111223336668800000";
    // char *ref = "22335555555666678890000001";
    // char *src = "481234781";
    // char *ref = "4411327431";
    // char *src = "2222221";
    // char *ref = "133333";
    // char *src = "1222222";
    // char *ref = "1";
    // char *src = "1";
    // char *ref = "333312222";
    char *src = "中文匹配符号。";
    char *ref = "看中文的匹配，还有符号";


    uint32_t maxLine = strlen(src) + 1;
    uint32_t maxColumn = strlen(ref) + 1;

    MatrixNode *matrix = new MatrixNode[maxLine*maxColumn];

    compareMatrix(matrix,(uint8_t*)src,(uint8_t*)ref,maxLine,maxColumn);
    dumpMatrixValue(matrix,maxLine,maxColumn,src,ref);
    MatrixNode *m = getOutputMatrix(matrix,maxLine,maxColumn);
    outputMatrix(m,"/Users/liangzhen/compare.html");
}
void TxtComparator::compareMatrix(MatrixNode *matrix,const uint8_t *src,const uint8_t *ref,uint64_t maxLine,uint64_t maxColumn)
{
    int i,j;
    i = j = 0;
    // step 1. init matrix,matrix size [srcLen + 1][refLen + 1]
    memset(matrix,0,sizeof(MatrixNode) * maxLine * maxColumn);
    for(i = 1; i < maxLine; i++)
    {
	(matrix + i * maxColumn) -> value = src[i-1]; // :为0列赋初值，解决目标文件起始为add操作时无法输出add的内容的bug
    }

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
static void moveDiag(TxtComparator::MatrixNode * matrix,uint64_t *pi,uint64_t *pj,uint64_t maxColumn)
{
    uint64_t i = *pi;
    uint64_t j = *pj;

    LOGD("move diag %d %d %d",(matrix + i * maxColumn + j) -> type,i,j);
    (matrix + (i-1) * maxColumn + j-1) -> nextNode = (matrix + i * maxColumn + j);
    // 节点类型由外部修改 key or modify
    *pi = i - 1;
    *pj = j - 1;
}
static void moveUp(TxtComparator::MatrixNode * matrix,uint64_t *pi,uint64_t *pj,uint64_t maxColumn)
{
    uint64_t i = *pi;
    uint64_t j = *pj;
    
    (matrix + i * maxColumn + j) -> type = TxtComparator::MATRIX_NODE_TYPE_ADD;
    LOGD("move up %d %d %d",(matrix + i * maxColumn + j) -> type,i,j);
    (matrix + (i-1) * maxColumn + j) -> nextNode = (matrix + i * maxColumn + j);

    *pi = i - 1;
}
static void moveLeft(TxtComparator::MatrixNode * matrix,uint64_t *pi,uint64_t *pj,uint64_t maxColumn)
{
    uint64_t i = *pi;
    uint64_t j = *pj;

    (matrix + i * maxColumn + j) -> type = TxtComparator::MATRIX_NODE_TYPE_DEL;
    LOGD("move left %d %d %d",(matrix + i * maxColumn + j) -> type,i,j);
    (matrix + i * maxColumn + j-1) -> nextNode = (matrix + i * maxColumn + j);

    *pj = j - 1;
}
TxtComparator::MatrixNode *TxtComparator::getOutputMatrix(MatrixNode *matrix,uint64_t maxLine,uint64_t maxColumn)
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
		    (matrix + i * maxColumn + j) -> type = MATRIX_NODE_TYPE_KEY;
		    moveDiag(matrix,&i,&j,maxColumn);break;
		case 2:
		    moveUp(matrix,&i,&j,maxColumn);break;
		case 3:
		    moveLeft(matrix,&i,&j,maxColumn);break;
		case 4:
		    if(i > j)
			moveUp(matrix,&i,&j,maxColumn);
		    else
			moveLeft(matrix,&i,&j,maxColumn);
		    break;
		default:
		    // do nothing
		    LOGE("never come here");
		}	
	    }else{
		switch(max)
		{
		case 1:
		    (matrix + i * maxColumn + j)->type = MATRIX_NODE_TYPE_MODIFY;
		    moveDiag(matrix,&i,&j,maxColumn);break;
		case 2:
		    moveUp(matrix,&i,&j,maxColumn);break;
		case 3:
		    moveLeft(matrix,&i,&j,maxColumn);break;
		case 4:
		    if(i > j)
			moveUp(matrix,&i,&j,maxColumn);
		    else
			moveLeft(matrix,&i,&j,maxColumn);
		    break;
		case 5:
		    if(i < j)
			moveLeft(matrix,&i,&j,maxColumn);
		    else{
			(matrix + i * maxColumn + j)->type = MATRIX_NODE_TYPE_MODIFY;
			moveDiag(matrix,&i,&j,maxColumn);
		    }
		    break;
		case 6:
		    if(i < j)
			moveUp(matrix,&i,&j,maxColumn);
		    else{
			(matrix + i * maxColumn + j)->type = MATRIX_NODE_TYPE_MODIFY;
			moveDiag(matrix,&i,&j,maxColumn);
		    }
		    break;
		case 7:
		    if(i < j)
			moveLeft(matrix,&i,&j,maxColumn);
		    else if(i > j)
			moveUp(matrix,&i,&j,maxColumn);
		    else{
			(matrix + i * maxColumn + j)->type = MATRIX_NODE_TYPE_MODIFY;
			moveDiag(matrix,&i,&j,maxColumn);
		    }
		    break;
		default:
		    // do nothing
		    LOGE("never come here");
		}
	    }
	}else if(i > 0){
	    // j must be 0,just move up
	    moveUp(matrix,&i,&j,maxColumn);
	}else if(j > 0){
	    // i must be 0,just move left
	    moveLeft(matrix,&i,&j,maxColumn);
	}else{
	    // first node
	    break;
	}
    }while(true);

    return matrix->nextNode;
}
bool TxtComparator::outputMatrix(MatrixNode *matrix,const char *outputFile)
{
    FILE * fp = fopen(outputFile,"wb+");
    if(NULL == fp)
    {
	LOGE("open output file err!");
	return false;
    }

    // 写入显示样式
    char css[] = "\
<style type=\"text/css\"> \
.key{color:black;}\
.add{color:blue;}\
.mod{color:red;}\
</style>";
    fwrite(css,sizeof(css),1,fp);
    // :end
    MatrixNode *tmp = matrix;
    char buf[sizeof("<span class=\"key\"></span>") + 1];
    memset(buf,0,sizeof(buf));
    while(NULL != tmp)
    {
	switch(tmp->type)
	{
	case MATRIX_NODE_TYPE_KEY:
	    fwrite("<span class=\"key\">",sizeof("<span class=\"key\">"),1,fp);
	    fwrite(&(tmp->value),sizeof(uint8_t),1,fp);
	    fwrite("</span>",sizeof("</span>"),1,fp);
	    break;
	case MATRIX_NODE_TYPE_ADD:
	    fwrite("<span class=\"add\">",sizeof("<span class=\"add\">"),1,fp);
	    fwrite(&(tmp->value),sizeof(uint8_t),1,fp);
	    fwrite("</span>",sizeof("</span>"),1,fp);
	    break;
	case MATRIX_NODE_TYPE_DEL:
	    fwrite("<span class=\"del\">_</span>",sizeof("<span class=\"del\">_</span>"),1,fp);break;
	case MATRIX_NODE_TYPE_MODIFY:
	    fwrite("<span class=\"mod\">",sizeof("<span class=\"mod\">"),1,fp);
	    fwrite(&(tmp->value),sizeof(uint8_t),1,fp);
	    fwrite("</span>",sizeof("</span>"),1,fp);
	    break;
	default:
	    LOGE("unkown output node type!%d",tmp->type);
	    fclose(fp);
	    return false;
	}
	tmp = tmp -> nextNode;
    }
    fclose(fp);
    return true;
}
void TxtComparator::dumpMatrixValue(MatrixNode *matrix,uint64_t maxLine,uint64_t maxColumn,char *src,char *ref)
{
    int i,j;
    printf("0|");
    printf("%3d",0);
    for( i = 0; i < maxColumn - 1; i++)
    {
	printf("%3c",ref[i]);
    }
    printf("\r\n");
    for( i = 0; i < maxColumn; i++)
    {
	printf("---");
    }
    printf("--");
    printf("\r\n");
    for(i = 0; i < maxLine; i++)
    {
	if(i == 0)
	    printf("0|");
	else
	    printf("%c|",src[i-1]);
	for(j = 0; j < maxColumn; j++)
	{
	    printf("%3d",(matrix + i * maxColumn +j) -> maxSubsequenceLen);
	}
	printf("\r\n");
    }
}
