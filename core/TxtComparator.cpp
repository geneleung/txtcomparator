#include "stddefs.h"
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
    //
    LOGD("compare two files!");
}
void TxtComparator::compareMatrix(MatrixNode *matrix,const uint8_t *src,const uint8_t *ref,uint64_t srcLen,uint64_t refLen)
{
    // 
}
void TxtComparator::outputMatrix(MatrixNode *matrix,uint64_t maxLine,uint64_t maxColumn,const uint8_t *outputFile)
{
    // 
}


















