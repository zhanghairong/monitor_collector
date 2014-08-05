#include "watchdogthread.h"
#include "collectcommon.h"
#include "mongothread.h"
#include "filehelper.h"
#include "stringhelper.h"
#include "configmanager.h"
#include <fstream>
#include <boost/unordered_map.hpp>
#include <vector>
#include <ape/loghelper.h>
#include <ape/dirreader.h>

namespace ape {
namespace monitor {
#define WATCH_DOG_SELF_CHECK_INTERVAL 30000 //30s
#define WATCH_DOG_DO_FLUSHLOG_INTERVAL 30000 //30s

#ifndef MAX_PATH_LEN
#define  MAX_PATH_LEN 256
#endif

WatchDogThread *WatchDogThread::sminstance_ = NULL;
WatchDogThread *WatchDogThread::GetInstance() {
  if (!sminstance_) {
    sminstance_ = new WatchDogThread();
  }
  return sminstance_;
}

WatchDogThread::WatchDogThread() :
  timerselfcheck_(this, WATCH_DOG_SELF_CHECK_INTERVAL, boost::bind(&WatchDogThread::DoSelfCheck, this), ape::common::CThreadTimer::TIMER_CIRCLE),
  timerchecklog_(this, WATCH_DOG_DO_FLUSHLOG_INTERVAL, boost::bind(&WatchDogThread::DoCheckLog, this), ape::common::CThreadTimer::TIMER_CIRCLE)
{
    funcmap_[EN_WATCHDOG_MSG_ACTION_FAIL] = &WatchDogThread::DoActionFail;
    funcmap_[EN_WATCHDOG_MSG_ERROR_FAIL] = &WatchDogThread::DoErrorFail;
}
WatchDogThread::~WatchDogThread() {
    BS_XLOG(XLOG_DEBUG,"WatchDogThread::%s\n",__FUNCTION__);
}
void WatchDogThread::StartInThread() {
    BS_XLOG(XLOG_DEBUG,"WatchDogThread::%s\n",__FUNCTION__);
    XLOG_REGISTER(SELF_CHECK_MODULE, "selfcheck");
    timerselfcheck_.Start();
    timerchecklog_.Start();
}
void WatchDogThread::StopInThread() {
    BS_XLOG(XLOG_DEBUG,"WatchDogThread::%s\n",__FUNCTION__);
}
void WatchDogThread::Deal(void *pdata) {
    SWatchDogMsg *pmsg = (SWatchDogMsg *)pdata;
    (this->*(funcmap_[pmsg->type]))(pmsg);
    delete pmsg;
}
void WatchDogThread::OnActionFail(const SCurveInfo &info) {
    SActionFailMsg *msg = new SActionFailMsg();
    msg->info = info;
    PutQ(msg);
}
void WatchDogThread::OnErrorFail(const SErrorInfo &info) {
	BS_XLOG(XLOG_DEBUG,"WatchDogThread::%s, [%s]\n", __FUNCTION__, info.ToString().c_str());
    SErrorFailMsg *msg = new SErrorFailMsg;
    msg->info = info;
    PutQ(msg);
}
void WatchDogThread::DoActionFail(SWatchDogMsg *msg) {
    SActionFailMsg *pmsg = (SActionFailMsg *)msg;
    const std::string &file = ConfigManager::GetInstance()->GetActionRecordFile();
	WriteFile(file, &actionlog_, pmsg->info.ToString());
}
void WatchDogThread::DoErrorFail(SWatchDogMsg *msg) {
    SErrorFailMsg *pmsg = (SErrorFailMsg *)msg;
    const std::string &file = ConfigManager::GetInstance()->GetErrorRecordFile();
	WriteFile(file, &errorlog_, pmsg->info.ToString());
}
void WatchDogThread::DoSelfCheck() {
}
void WatchDogThread::DoCheckLog() {
    BS_XLOG(XLOG_DEBUG,"WatchDogThread::%s\n", __FUNCTION__);
    const std::string &actionfile = ConfigManager::GetInstance()->GetActionRecordFile();
    const std::string &errorfile = ConfigManager::GetInstance()->GetErrorRecordFile();
	
	FlushBufferToFile(actionfile, &actionlog_);
	FlushBufferToFile(errorfile, &errorlog_);
    
	if (ape::common::ExistFile(actionfile.c_str())) {
		std::string strretryfile = actionfile + ".retry";
        rename(actionfile.c_str(), strretryfile.c_str());
		DealActionFile(strretryfile);
    }
	if (ape::common::ExistFile(errorfile.c_str())) {
		std::string strretryfile = errorfile + ".retry";
        rename(errorfile.c_str(), strretryfile.c_str());
		DealErrorFile(strretryfile);
    }
}
void WatchDogThread::DealActionFile(const std::string &file) {
    BS_XLOG(XLOG_DEBUG,"WatchDogThread::%s, file[%s]\n", __FUNCTION__, file.c_str());
	if (!ape::common::ExistFile(file.c_str())) {
        return;
    }
    std::ifstream fin;
    fin.open(file.c_str());
    if( !fin ) {
        BS_XLOG(XLOG_ERROR, "WatchDogThread::%s, open file[%s] failed\n", __FUNCTION__, file.c_str());
        return;
    }
    std::string line;
    while (getline(fin, line)) {
		SCurveInfo info;
		if (0 == info.InitFromString(line)) {
			CMongoThreadGroup::GetInstance()->OnAction(info);
		}
    }
    fin.close();
    remove(file.c_str());
}
void WatchDogThread::DealErrorFile(const std::string &file) {
    BS_XLOG(XLOG_DEBUG,"WatchDogThread::%s, file[%s]\n", __FUNCTION__, file.c_str());
	if (!ape::common::ExistFile(file.c_str())) {
        return;
    }
    std::ifstream fin;
    fin.open(file.c_str());
    if( !fin ) {
        BS_XLOG(XLOG_ERROR, "WatchDogThread::%s, open file[%s] failed\n", __FUNCTION__, file.c_str());
        return;
    }
    std::string line;
    while (getline(fin, line)) {
		SErrorInfo info;
		if (0 == info.InitFromString(line)) {
			CMongoThreadGroup::GetInstance()->OnError(info);
		}
    }
    fin.close();
    remove(file.c_str());
}

void WatchDogThread::WriteFile(const std::string &file, SLogBuf *buf, const std::string &str) {
    if (str.length() + 8 > MAX_WATCH_LOG_BUFFER_SIZE) {
        FlushBufferToFile(file, buf);
        ape::common::WriteToFile(file, str.c_str(), str.length());
        return;
    }
    if (str.length() + buf->loc + 8 > MAX_WATCH_LOG_BUFFER_SIZE) {
        FlushBufferToFile(file, buf);
    }
    memcpy(buf->buf + buf->loc, str.c_str(), str.length());
    buf->loc += str.length();
    *(buf->buf + buf->loc) = 0;
    if(buf->loc + 128 > MAX_WATCH_LOG_BUFFER_SIZE)  {
        FlushBufferToFile(file, buf);
    }
}
void WatchDogThread::FlushBufferToFile(const std::string &file, SLogBuf *buf) {
    if (buf->loc > 0) {
        ape::common::WriteToFile(file, buf->buf, buf->loc);
        buf->loc = 0;
        buf->buf[buf->loc] = 0;
    }
}

}
}
