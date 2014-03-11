#include "stddefs.h"

#define LOG_TAG "TEST_SP"

using namespace android;

class Obj:public virtual RefBase
{
public:
    Obj(int n):m(n)
    {
        LOGD("%d obj construct!",m);
    }
    virtual ~Obj()
    {
        LOGD("destroy %d obj",m);
    }
    virtual void onFirstRef()
    {
        LOGD("%d first time construct",m);
    }
private:
    int m;
};
int testSP()
{
    {				// block 1
        sp<Obj> o1 = NULL;
        {			     // block 2
            sp<Obj> o2 = new Obj(2); // when exit block 2,o2 should be destroy!
            sp<Obj> o3 = new Obj(3);

            o1 = o3;		// when exit block 2,o3 should not be destroy!
            LOGD("leaving block 2...");
        }
        LOGD("leaving block 1..."); // when exit block 1,o3 should be destroy!
    }
    return 0;
}















