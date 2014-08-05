#ifndef _APE_MONITOR_BUSINESS_HANDLE_H_
#define _APE_MONITOR_BUSINESS_HANDLE_H_
#include <ape/events.h>
#include <ape/netservice.h>
#include <vector>
#include <string>
#include "collectcommon.h"

namespace ape {
namespace monitor {

class CBusinessHandle : public ape::net::CHandle {
 public:
    typedef void (CBusinessHandle::*DealUrlFunc)(ape::message::SHttpMessage *, const std::string &);
    CBusinessHandle();
    virtual ~CBusinessHandle() {}
    virtual void StartInThread(int threadid, ape::net::CNetService *service, ape::common::CTimerManager *owner);
    virtual void StopInThread(int threadid) {}
    virtual void OnEvent(int threadid, void *event);
    void Dump();

 private:
    void DealHttpRequest(ape::message::SHttpMessage *message);
    void DoSendHttpResponse(ape::message::SHttpMessage *request, int code, const std::string &body);
    
    void OnTest(ape::message::SHttpMessage *message, const std::string &param);
    void OnStatAction(ape::message::SHttpMessage *message, const std::string &param);
    void OnErrorReport(ape::message::SHttpMessage *message, const std::string &param);
    
    void DecodeErrorReportParam(const char *str, int len, SErrorInfo *info);
    void DecodeActionParam(const char *str, int len, std::string *projectid, std::string *ip, std::vector<std::string> *actions);
 private:
    int threadid_;
    ape::net::CNetService *service_;
    ape::common::CTimerManager *timerowner_;
    
    boost::unordered_map<std::string, DealUrlFunc> mapurlfunc_;
};
}
}
#endif
