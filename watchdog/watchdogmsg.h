#ifndef APE_MONITOR_WATCH_DOG_MSG_THREAD_H
#define APE_MONITOR_WATCH_DOG_MSG_THREAD_H
#include <string>
#include "collectcommon.h"

namespace ape {
namespace monitor {
typedef enum {
    EN_WATCHDOG_MSG_ACTION_FAIL = 0,
    EN_WATCHDOG_MSG_ERROR_FAIL,
    EN_WATCHDOG_MSG_ALL
}EWatchDogMsgType;
typedef struct stWatchDogMsg {
    EWatchDogMsgType type;
    stWatchDogMsg(EWatchDogMsgType t) : type(t) {}
    virtual ~stWatchDogMsg() {}
}SWatchDogMsg;
typedef struct stActionFailMsg : public SWatchDogMsg {
    SCurveInfo info;
    stActionFailMsg() : stWatchDogMsg(EN_WATCHDOG_MSG_ACTION_FAIL) {}
    virtual ~stActionFailMsg() {}
}SActionFailMsg;
typedef struct stErrorFailMsg : public SWatchDogMsg {
    SErrorInfo info;
    stErrorFailMsg() : stWatchDogMsg(EN_WATCHDOG_MSG_ERROR_FAIL) {}
    virtual ~stErrorFailMsg() {}
}SErrorFailMsg;

typedef enum {
    E_Check_Action = 0,
    E_Check_Error
}ECheckLogType;
}
}

#endif