#include "ui/compareThread.h"

CompareThread::CompareThread(android::sp<TxtComparator> c,const char *s,const char *r,const char *o)
    :comparator(NULL),src(NULL),ref(NULL),out(NULL)
{
    comparator = c;
    src = s;
    ref = r;
    out = o;
}
CompareThread::~CompareThread()
{
    comparator = NULL;		// 释放引用
}
void CompareThread::run()
{
    comparator->compare(src,ref,out);
}


















