#ifndef _APE_MONITOR_COLLECT_MSG_H_
#define _APE_MONITOR_COLLECT_MSG_H_
#include <vector>
#include <string>
#include "collectcommon.h"

namespace ape {
namespace monitor {
typedef enum {
    E_Collect_Stat_Action = 0,
    E_Collect_Error_Report,
    E_Collect_All
}ECollectType;

typedef struct stBaseCollectMsg {
    ECollectType type;
    stBaseCollectMsg(ECollectType t) : type(t) {}
    virtual ~stBaseCollectMsg(){}
}SBaseCollectMsg;
typedef struct stStatActionMsg : public SBaseCollectMsg {
    std::string projectid;
    std::string ip;
    std::vector<std::string> actions;
    stStatActionMsg() : SBaseCollectMsg(E_Collect_Stat_Action) {}
    virtual ~stStatActionMsg(){}
}SStatActionMsg;
typedef struct stErrorReportMsg : public SBaseCollectMsg {
    SErrorInfo info;
    stErrorReportMsg() : SBaseCollectMsg(E_Collect_Error_Report) {}
    virtual ~stErrorReportMsg(){}
}SErrorReportMsg;
}
}
#endif
