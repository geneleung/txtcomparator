#include "stddefs.h"
#include <stdio.h>

#define MODULE_NAME "TEST_DATA_TYPES"

int testMechineType()
{
    LOGD("int8_t:%lu \r\n",sizeof(int8_t));
    LOGD("int16_t:%lu \r\n",sizeof(int16_t));
    LOGD("int32_t:%lu \r\n",sizeof(int32_t));
    LOGD("int64_t:%lu \r\n",sizeof(int64_t));


    LOGD("uint8_t:%lu \r\n",sizeof(uint8_t));
    LOGD("uint16_t:%lu \r\n",sizeof(uint16_t));
    LOGD("uint32_t:%lu \r\n",sizeof(uint32_t));
    LOGD("uint64_t:%lu \r\n",sizeof(uint64_t));
    
    return 0;
}
int main()
{
    return testMechineType();
}






