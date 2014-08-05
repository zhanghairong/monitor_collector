#ifndef _APE_COMMON_MONGO_THREAD_H_
#define _APE_COMMON_MONGO_THREAD_H_
#include <vector>
#include <ape/msgtimerthread.h>
#include <ape/threadtimer.h>
#include "mongoclient.h"
#include "dbmsg.h"
#include "collectcommon.h"

namespace ape {
namespace monitor {
class CMongoThread : public ape::common::MsgTimerThread {
 public:
    typedef void (CMongoThread::*MsgFunc)(SDbMsg *);
    CMongoThread(ape::common::MsgQueuePrio *queue, int no);
    virtual ~CMongoThread();
    virtual void StartInThread();
    virtual void StopInThread();
    virtual void Deal(void *data);
    void GetSelfCheck(bool &isalive);
 private:
    void DoConnect(SDbMsg *data);
    void DoAction(SDbMsg *data);
    void DoError(SDbMsg *data);
    void DoSelfCheck();
    CMongoClient *GetClient();
 private:
    int id_;
    volatile bool isalive_;
    boost::unordered_map<std::string, CMongoClient *> clients_;
    MsgFunc funcmap_[E_DB_All];
    ape::common::CThreadTimer tmselfcheck_;
};

class CMongoThreadGroup {
 public:
    static CMongoThreadGroup *GetInstance();
    static void Release();
    virtual ~CMongoThreadGroup();
    void Start(int threads = 5);
    void Stop();
    void OnConnect(const std::string &addr);
    void OnAction(const SCurveInfo &info);
    void OnError(const SErrorInfo &info);
 private:
    CMongoThreadGroup();
 private:
    std::string basedb;
    static CMongoThreadGroup *sminstance_;
    ape::common::MsgQueuePrio queue_;
    std::vector<CMongoThread *> threads_;
};
}
}
#endif
