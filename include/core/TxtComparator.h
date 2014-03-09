#ifndef TXT_COMPARATOR_H
#define TXT_COMPARATOR_H

#include "stddefs.h"
class TxtComparator:public virtual RefBase
{
public:
    TxtComparator();
    virtual ~TxtComparator();
    void compare(const uint8_t *srcFile,const uint8_t *refFile);
private:
    class MatrixNode:public virtual RefBase
    {
    public:
	MatrixNode();
	virtual ~MatrixNode();
	// :如果是匹配节点，增加1个权重，同时增加所有前面与它相邻的节点权重，最终权重为相邻匹配的个数
	void incWeight();
    private:
	uint64_t maxSubsequenceLen;//:max file size 2^64 = 2^14 PB,very large,that is enough~
	uint64_t weight; //:相邻匹配字符数，用以尽量匹配最长的字串
	sp<MatrixNode> preNode; //:前一个相邻匹配节点，用以修改节点权重
    };
    void comareMatrix(sp<MatrixNode> matrixNode,const uint8_t *src,const uint8_t *ref，uint64_t srcLen,uint64_t refLen);
    void outputMatrix(sp<MatrixNode> matrixNode,uint8_t *outputFile);
private:
    FILE *matrixFile;
};

#endif //:TXT_COMPARATOR_H



















