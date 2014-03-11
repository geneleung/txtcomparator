#ifndef COMPARE_THREAD_H
#define COMPARE_THREAD_H

#include <QThread>
#include "stddefs.h"
#include "core/TxtComparator.h"

class CompareThread : public QThread
{
    Q_OBJECT

public:
    CompareThread(android::sp<TxtComparator> c,const char *s,const char *r,const char *o);
    ~CompareThread();

    void run();
    
private:
    android::sp<TxtComparator> comparator;
    const char *src;
    const char *ref;
    const char *out;
};

#endif

















