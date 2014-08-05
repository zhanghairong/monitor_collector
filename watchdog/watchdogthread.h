#ifndef APE_MONITOR_WATCH_DOG_THREAD_H
#define APE_MONITOR_WATCH_DOG_THREAD_H
#include <string>
#include <map>
#include <vector>
#include <cstdio>
#include <ape/threadtimer.h>
#include <ape/msgtimerthread.h>
#include "watchdogmsg.h"

namespace ape {
namespace monitor {


#ifndef MAX_WATCH_LOG_BUFFER_SIZE
#define MAX_WATCH_LOG_BUFFER_SIZE 102400
#endif

class WatchDogThread: public ape::common::MsgTimerThread {
    typedef struct stLogBuf {
        char buf[MAX_WATCH_LOG_BUFFER_SIZE];
        int loc;
        stLogBuf() : loc(0) { memset(buf, 0, MAX_WATCH_LOG_BUFFER_SIZE);}
    }SLogBuf;
    typedef void(WatchDogThread::*MsgFunc)(SWatchDogMsg *);
  public:
    static WatchDogThread *GetInstance();
    virtual ~WatchDogThread();
    virtual void StartInThread();
    virtual void StopInThread();
    virtual void Deal(void *pdata);
    int Init(const std::string &failfile);
  public:
    void OnErrorFail(const SErrorInfo &info);
    void OnActionFail(const SCurveInfo &info);
  private:
    void DoActionFail(SWatchDogMsg *msg);
    void DoErrorFail(SWatchDogMsg *msg);
    WatchDogThread();
    void DoSelfCheck();
    void DoCheckLog();
	
	void DealErrorFile(const std::string &file);
	void DealActionFile(const std::string &file);
	void WriteFile(const std::string &file, SLogBuf *buf, const std::string &str);
	void FlushBufferToFile(const std::string &file, SLogBuf *buf);
	
  private:
    static WatchDogThread *sminstance_;
    ape::common::CThreadTimer timerselfcheck_;
    ape::common::CThreadTimer timerchecklog_;
    SLogBuf actionlog_;  
    SLogBuf errorlog_;  
    MsgFunc funcmap_[EN_WATCHDOG_MSG_ALL];
};

}
}
#endif