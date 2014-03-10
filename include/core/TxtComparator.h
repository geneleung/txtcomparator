#ifndef TXT_COMPARATOR_H
#define TXT_COMPARATOR_H

#include "stddefs.h"
class TxtComparator:public virtual android::RefBase
{
public:
    TxtComparator();
    virtual ~TxtComparator();
    bool compare(const char *srcFile,const char *refFile,const char *outputFile);
    void stop();
    uint32_t getPercent();
public:
    enum {
        MATRIX_NODE_TYPE_NONE  = 0,
        MATRIX_NODE_TYPE_KEY,
        MATRIX_NODE_TYPE_ADD,
	MATRIX_NODE_TYPE_DEL,
	MATRIX_NODE_TYPE_MODIFY,
    };
    // 方便处理大文件，直接使用内存映射
    struct MatrixNode
    {
	uint64_t maxSubsequenceLen;//:max file size 2^64 = 2^14 PB,very large,that is enough~
	// uint64_t line;
	// uint64_t column;
	MatrixNode *nextNode; //:下一个输出节点
	uint32_t type;	      	// :放末尾，32位机对齐
	uint8_t value;
    };
private:
    /*
     *matrix指针所指空间比较足够大，此函数不做越界检查
     * 生成对比矩阵，每个矩阵节点包含对应最大公共字串和权重
     */
    bool compareMatrix(MatrixNode *matrix,const uint8_t *src,const uint8_t *ref,uint64_t maxLine,uint64_t maxColumn);
    /*
     *根据对比矩阵输出对比文件
     */
    MatrixNode *getOutputMatrix(MatrixNode *matrix,uint64_t maxLine,uint64_t maxColumn);
    bool outputMatrix(MatrixNode *matrix,FILE *fp);
    // :for test
    void dumpMatrixValue(MatrixNode *matrix,uint64_t maxLine,uint64_t maxColumn,char *src,char *ref);
    // :end

private:
    //: 以下三个成员虽然都会在不同的线程种操作，但是只有单个线程写，单个线程读，无需加锁处理
    bool isRuning;
    uint64_t totalSize;
    uint64_t currentSize;
};

#endif //:TXT_COMPARATOR_H



















