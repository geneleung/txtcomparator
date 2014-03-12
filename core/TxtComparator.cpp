#include <fcntl.h>
#include <sys/stat.h>
//#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include "stddefs.h"
#include "core/TxtComparator.h"

#define LOG_TAG "TXT_COMPARATOR"

TxtComparator::TxtComparator()
    :isRuning(true),totalSize(0),currentSize(0)
{
    //
}
TxtComparator::~TxtComparator()
{
    //
    LOGD("destroy TxtComparator!");
}
void TxtComparator::stop()
{
    isRuning = false;
}
uint32_t TxtComparator::getPercent()
{
    if(0 == totalSize)
        return 0;
    return currentSize * 100 /totalSize;
}
static void *mapFile(int fd,uint64_t size,bool readOrWrite/*true read,false write*/)
{
    return mmap(NULL,size,readOrWrite?PROT_READ:PROT_WRITE,MAP_SHARED,fd,0);
}
static void unmapFile(void *ptr,uint64_t size)
{
    if(0 != munmap(ptr,size))
        LOGE("unmap file faild!");
}
bool TxtComparator::compare(const char *srcFile,const char *refFile,const char *outputFile)
{
    bool ret = true;
    uint64_t maxLine = 0;
    uint64_t maxColumn = 0;
    // step 1. open file ,and check
    int fpSrc = open(srcFile,O_RDONLY);
    int fpRef = open(refFile,O_RDONLY);
    char *tmpFile = new char[strlen(outputFile)+1+4];
    tmpFile = strcpy(tmpFile,outputFile);
    tmpFile = strcat(tmpFile,".tmp");
    LOGD("%s",tmpFile);
    int fpMatrix = open(tmpFile,O_RDWR|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    //int fpMatrix = open(tmpFile,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);//使用WRONLY打开文件会导致mmap失败

    delete[] tmpFile;
    FILE * fpOut = fopen(outputFile,"wb+");

    
    do{
        if((fpSrc < 0) || (fpRef < 0) || (fpMatrix < 0) || (NULL == fpOut))
        {
            LOGE("open file err!%d,%d,%d",fpSrc,fpRef,fpMatrix);
            ret = false;
            break;
        }
        // step 2. get file len
        struct stat status;
        if(0 != fstat(fpSrc, &status))
        {
            LOGE("can not get file size!");
            ret = false;
            break;
        }
        maxLine = status.st_size + 1; // TODO:if ftell faild???
        if(0 != fstat(fpRef, &status))
        {
            LOGE("can not get file size!");
            ret = false;
            break;
        }
        maxColumn = status.st_size + 1;
        LOGD("maxLine %ld,maxColumn %ld",maxLine,maxColumn);

        totalSize = maxLine * maxColumn + (maxLine + maxColumn) * 2; // matrix + 回溯 + 输出

        lseek(fpMatrix,maxLine*maxColumn*sizeof(MatrixNode)-1,SEEK_SET);
        write(fpMatrix,"",1);

        // step 3.compare len
        // 对比时尽量使用内存对比，因为需要索引访问内存数据，如果使用文件映射的方式，需要io，索引访问效率会降低
        // 但是当需要对比的文件很大时，打开文件需要用的时间已经和索引访问和对比的时间在同一个数量级，此时使用文件映射的方式，时间主要消耗在对比操作上
        if(0 == maxLine -1)
        {
            // delete all
            fwrite("<span class=\"del\">",sizeof("<span class=\"del\">"),1,fpOut);
            maxColumn --;
            while(maxColumn > 0)
            {
                fwrite("_",sizeof("_"),1,fpOut);
                maxColumn --;
            }
            fwrite("</span>",sizeof("</span>"),1,fpOut);
            break;		// close file and return
        }
        if(0 == maxColumn - 1)
        {
            // add all
            fwrite("<span class=\"add\">",sizeof("<span class=\"add\">"),1,fpOut);
            char *src = static_cast<char*>(mapFile(fpSrc,maxLine - 1,true));
            if(MAP_FAILED == src)
            {
                LOGE("map src file failed!");
                ret = false;
                break;
            }
            while(--maxLine > 0)
                fwrite(src++,1,1,fpOut);
            fwrite("</span>",sizeof("</span>"),1,fpOut);
            fflush(fpOut);
            unmapFile(src,maxLine - 1);
            break;		// close file and return
        }

        // compare by memery
        char *src = static_cast<char*>(mapFile(fpSrc,maxLine - 1,true));
        char *ref = static_cast<char*>(mapFile(fpRef,maxColumn - 1,true));
        MatrixNode *matrix = static_cast<MatrixNode*>(mapFile(fpMatrix,maxLine*maxColumn*sizeof(MatrixNode),false));

        if((MAP_FAILED == src)||(MAP_FAILED == ref)||(MAP_FAILED == matrix))
        {
            LOGE("no memery for compare!,%p,%p,%p",src,ref,matrix);
            LOGE("error: %s(errno: %d)",strerror(errno),errno);
            unmapFile((void*)src,maxLine - 1);
            unmapFile((void*)ref,maxColumn - 1);
            unmapFile((void*)matrix,maxLine*maxColumn);
            ret = false;
            break;
        }
        if(compareMatrix(matrix,(uint8_t*)src,(uint8_t*)ref,maxLine,maxColumn))
            ret = outputMatrix(getOutputMatrix(matrix,maxLine,maxColumn),fpOut);
        else{
            // 比较过程中被中断
            ret = false;
            break;
        }
        fflush(fpOut);
        unmapFile((void*)src,maxLine - 1);
        unmapFile((void*)ref,maxColumn - 1);
        unmapFile((void*)matrix,maxLine*maxColumn);
        break;
    }while(true);
    close(fpSrc);
    close(fpRef);
    close(fpMatrix);
    fclose(fpOut);
    return ret;
}
bool TxtComparator::compareMatrix(MatrixNode *matrix,const uint8_t *src,const uint8_t *ref,uint64_t maxLine,uint64_t maxColumn)
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
            if(!isRuning)
                return false;
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
            currentSize ++;
        }
    }
    return true;
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
        if(!isRuning)
            return NULL;
        currentSize ++;
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
bool TxtComparator::outputMatrix(MatrixNode *matrix,FILE *fp)
{
    // FILE * fp = fopen(outputFile,"wb+");
    // if(NULL == fp)
    // {
    // 	LOGE("open output file err!");
    // 	return false;
    // }

    // 写入显示样式
    char css[] = "\
            <style type=\"text/css\"> \
            .key{color:black;}\
            .add{color:blue;}\
            .mod{color:red;}\
            .del{color:red;}\
            </style>";
            fwrite(css,sizeof(css),1,fp);
    // :end
    MatrixNode *tmp = matrix;
    char buf[sizeof("<span class=\"key\"></span>") + 1];
    memset(buf,0,sizeof(buf));
    while((NULL != tmp) && isRuning)
    {
        switch(tmp->type)
        {
        case MATRIX_NODE_TYPE_KEY:
            fwrite("<span class=\"key\">",sizeof("<span class=\"key\">"),1,fp);
            do{
                fwrite(&(tmp->value),sizeof(uint8_t),1,fp);
                tmp = tmp -> nextNode;
            }while((NULL != tmp) && (MATRIX_NODE_TYPE_KEY == tmp -> type));
            fwrite("</span>",sizeof("</span>"),1,fp);
            break;
        case MATRIX_NODE_TYPE_ADD:
            fwrite("<span class=\"add\">",sizeof("<span class=\"add\">"),1,fp);
            do{
                fwrite(&(tmp->value),sizeof(uint8_t),1,fp);
                tmp = tmp -> nextNode;
            }while((NULL != tmp) && (MATRIX_NODE_TYPE_ADD == tmp -> type));
            fwrite("</span>",sizeof("</span>"),1,fp);
            break;
        case MATRIX_NODE_TYPE_DEL:
            fwrite("<span class=\"del\">",sizeof("<span class=\"del\">"),1,fp);
            do{
                fwrite("_",sizeof("_"),1,fp);
                tmp = tmp -> nextNode;
            }while((NULL != tmp) && (MATRIX_NODE_TYPE_DEL == tmp -> type));
            fwrite("</span>",sizeof("</span>"),1,fp);
            break;
        case MATRIX_NODE_TYPE_MODIFY:
            fwrite("<span class=\"mod\">",sizeof("<span class=\"mod\">"),1,fp);
            do{
                fwrite(&(tmp->value),sizeof(uint8_t),1,fp);
                tmp = tmp -> nextNode;
            }while((NULL != tmp) && (MATRIX_NODE_TYPE_MODIFY == tmp -> type));
            fwrite("</span>",sizeof("</span>"),1,fp);
            break;
        default:
            LOGE("unkown output node type!%d",tmp->type);
            fclose(fp);
            return false;
        }
        // tmp = tmp -> nextNode;
        currentSize ++;
    }
    // fclose(fp);
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
