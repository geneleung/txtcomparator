#ifndef TXT_COMPARATOR_H
#define TXT_COMPARATOR_H

#include "stddefs.h"
class TxtComparator:public virtual android::RefBase
{
public:
    TxtComparator();
    virtual ~TxtComparator();
    void compare(const uint8_t *srcFile,const uint8_t *refFile);
private:
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
	// uint8_t type;	      	// :是否是关键节点，关键节点即匹配节点
	uint64_t maxSubsequenceLen;//:max file size 2^64 = 2^14 PB,very large,that is enough~
	uint64_t weight; //:相邻匹配字符数，用以尽量匹配最长的字串
	MatrixNode *preNode; //:前一个相邻匹配节点，用以修改节点权重
	uint32_t type;	      	// :放末尾，32位机对齐
    };
    /*
     *matrix指针所指空间比较足够大，此函数不做越界检查
     * 生成对比矩阵，每个矩阵节点包含对应最大公共字串和权重
     * TODO:utf-8字符对比
     */
    void compareMatrix(MatrixNode *matrix,const uint8_t *src,const uint8_t *ref,uint64_t srcLen,uint64_t refLen);
    /*
     *根据对比矩阵输出对比文件
     */
    void outputMatrix(MatrixNode *matrix,uint64_t maxLine,uint64_t maxColumn,const uint8_t *outputFile);
    // :for test
    void dumpMatrixValue(MatrixNode *matrix,uint64_t maxLine,uint64_t maxColumn);
    // :end

private:
    FILE *matrixFile;
};

#endif //:TXT_COMPARATOR_H


















