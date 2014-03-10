//#include <QCoreApplication>
#include "stddefs.h"
#include "core/TxtComparator.h"

int main(int argc, char *argv[])
{
    //QCoreApplication a(argc, argv);
    android::sp<TxtComparator> comparator = new TxtComparator();
    comparator->compare("/Users/liangzhen/testcompare/a.txt","/Users/liangzhen/testcompare/b.txt","/Users/liangzhen/testcompare/c.html");
    return 0;
    //return a.exec();
}
