#ifndef _APE_MONITOR_COLLECT_THREAD_H_
#define _APE_MONITOR_COLLECT_THREAD_H_
#include <ape/loghelper.h>
#include <ape/msgtimerthread.h>
#include <ape/threadtimer.h>
#include <vector>
#include <string>
#include <boost/unordered_map.hpp>
#include "collectmsg.h"

namespace ape {
namespace monitor {
typedef struct stCurveKey SCurveKey;
class CCollectThread : public ape::common::MsgTimerThread {
  public:
    typedef void (CCollectThread::*DoFunc)(SBaseCollectMsg *);
    static CCollectThread *GetInstance();
    virtual void StartInThread();
    virtual void StopInThread();
    virtual void Deal(void *data);
    virtual ~CCollectThread();
    virtual void Dump();
  public:
    void OnStatAction(const std::string &projectid, const std::string &ip, const std::vector<std::string> &actions);
    void OnErrorReport(const SErrorInfo &info);
    
  private:
    CCollectThread();
    void DoStatAction(SBaseCollectMsg *msg);
    void DoStatPage(SBaseCollectMsg *msg);
    void DoErrorReport(SBaseCollectMsg *msg);
    void DoFlushAction();
    void DoFlushError();
  private:
    static CCollectThread * sm_instance_;
    DoFunc funcmap_[E_Collect_All];
    
    boost::unordered_map<SCurveInfo, int> map_action_;
    boost::unordered_map<SErrorInfo, int> map_error_;
    
    ape::common::CThreadTimer tm_action_;
    ape::common::CThreadTimer tm_error_;
};
}
}
#endif
