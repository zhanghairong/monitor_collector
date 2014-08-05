#ifndef _APE_COMMON_DBMSG_H_
#define _APE_COMMON_DBMSG_H_
#include <string>
#include "collectcommon.h"

namespace ape {
namespace monitor {
typedef enum {
    E_Db_Connect = 0,
    E_Db_Action,
    E_Db_Error,
    E_DB_All
}EDbMsgType;
typedef struct stDbMsg {
    EDbMsgType type;
    stDbMsg(EDbMsgType t) : type(t) {}
    virtual ~stDbMsg() {}
}SDbMsg;
typedef struct stConnectDbMsg : public SDbMsg{
    std::string addr;
    stConnectDbMsg() : SDbMsg(E_Db_Connect) {}
    virtual ~stConnectDbMsg() {}
}SConnectDbMsg;
typedef struct stActionDbMsg : public SDbMsg{
    SCurveInfo curveinfo;
    stActionDbMsg() : SDbMsg(E_Db_Action) {}
    virtual ~stActionDbMsg() {}
}SActionDbMsg;
typedef struct stErrorDbMsg : public SDbMsg{
    SErrorInfo info;
    stErrorDbMsg() : SDbMsg(E_Db_Error) {}
    virtual ~stErrorDbMsg() {}
}SErrorDbMsg;
}
}
#endif
