#include "collectthread.h"
#include <ape/loghelper.h>
#include "collectcommon.h"
#include "mongothread.h"
#include "stringhelper.h"

namespace ape {
namespace monitor {
const unsigned int FLUSH_ACTION_INTERVAL = 10000;//300000; //5min
const unsigned int FLUSH_ERROR_INTERVAL = 10000; //10s

CCollectThread * CCollectThread::sm_instance_ = NULL;
CCollectThread * CCollectThread::GetInstance() {
    if(sm_instance_ == NULL)
        sm_instance_ = new CCollectThread;
    return sm_instance_;
}
CCollectThread::CCollectThread() :
    tm_action_(this, FLUSH_ACTION_INTERVAL, boost::bind(&CCollectThread::DoFlushAction, this), ape::common::CThreadTimer::TIMER_ONCE), 
    tm_error_(this, FLUSH_ERROR_INTERVAL, boost::bind(&CCollectThread::DoFlushError, this), ape::common::CThreadTimer::TIMER_CIRCLE) 
{
    funcmap_[E_Collect_Stat_Action] = &CCollectThread::DoStatAction;
    funcmap_[E_Collect_Error_Report] = &CCollectThread::DoErrorReport;
}
CCollectThread::~CCollectThread() {
}
void CCollectThread::StartInThread() {
    time_t now = time(NULL);
    struct tm localtm;
    localtime_r(&now, &localtm);
    unsigned int seconds = localtm.tm_hour * 60 * 60 + localtm.tm_min * 60 + localtm.tm_sec;
    tm_action_.SetInterval(FLUSH_ACTION_INTERVAL - ((seconds * 1000) % FLUSH_ACTION_INTERVAL));
    tm_action_.Start();
    tm_error_.Start();
}
void CCollectThread::StopInThread() {
    tm_action_.Stop();
    tm_error_.Stop();
}
void CCollectThread::Deal(void *data) {
    SBaseCollectMsg *msg = (SBaseCollectMsg *)data;
    (this->*(funcmap_[msg->type]))(msg);
    delete msg;
}
void CCollectThread::OnStatAction(const std::string &projectid, const std::string &ip, const std::vector<std::string> &actions) {
    SStatActionMsg *msg = new SStatActionMsg;
    msg->projectid = projectid;
    msg->ip = ip;
    msg->actions = actions;
    PutQ(msg);
}
void CCollectThread::OnErrorReport(const SErrorInfo &info) {
    SErrorReportMsg *msg = new SErrorReportMsg;
    msg->info = info;
    PutQ(msg);
}
void CCollectThread::DoStatAction(SBaseCollectMsg *msg) {
    SStatActionMsg *message = (SStatActionMsg *)msg;
    BS_XLOG(XLOG_DEBUG, "CCollectThread::%s, projectid[%s]\n", __FUNCTION__, message->projectid.c_str());
    std::vector<std::string>::iterator itr = message->actions.begin();
    for (; itr != message->actions.end(); ++itr) {
        //BS_XLOG(XLOG_TRACE, "CCollectThread::%s, action[%s]\n", __FUNCTION__, itr->c_str());
        std::vector<std::string > vec;
        ape::common::split(*itr, '|', &vec);
        if (vec.size() != 3) {
            BS_XLOG(XLOG_WARNING, "CCollectThread::%s, projectid[%s], illegal action[%s]\n", __FUNCTION__, 
                message->projectid.c_str(), itr->c_str());
            continue;
        }
        int count = atoi(vec[2].c_str());
        std::string &curveid = vec[0];
        std::string &page = vec[1];
        if (curveid.length() != 24 || count == 0) {
            BS_XLOG(XLOG_WARNING, "CCollectThread::%s, projectid[%s], curveid or count is illegal, action[%s]\n", __FUNCTION__, 
                message->projectid.c_str(), itr->c_str());
            continue;
        }
        SCurveInfo info(message->projectid, curveid, message->ip, page);
        
        boost::unordered_map<SCurveInfo, int>::iterator itrm = map_action_.find(info);
        if (itrm == map_action_.end()) {
            map_action_[info] = count;
        } else {
            itrm->second += count;
        }
    }
    //Dump();
}

void CCollectThread::DoErrorReport(SBaseCollectMsg *msg) {
    SErrorReportMsg *message = (SErrorReportMsg *)msg;
    SErrorInfo &info = message->info;
    BS_XLOG(XLOG_DEBUG, "CCollectThread::%s, projectid[%s]\n", __FUNCTION__, info.projectid.c_str());

    boost::unordered_map<SErrorInfo, int>::iterator itr = map_error_.find(info);
    if (itr == map_error_.end()) {
        map_error_[info] = 1;
    } else {
        itr->second += 1;
    }
}
void CCollectThread::DoFlushAction() {
    BS_XLOG(XLOG_DEBUG, "CCollectThread::%s\n", __FUNCTION__);
    time_t now = time(NULL);
    boost::unordered_map<SCurveInfo, int>::iterator itr = map_action_.begin();
    for (; itr != map_action_.end(); ++itr) {
        SCurveInfo info = itr->first;
        info.timestamp = now;
        info.count = itr->second;
        CMongoThreadGroup::GetInstance()->OnAction(info);
    }
    map_action_.clear();
    
    struct tm localtm;
    localtime_r(&now, &localtm);
    unsigned int seconds = localtm.tm_hour * 60 * 60 + localtm.tm_min * 60 + localtm.tm_sec;
    //BS_XLOG(XLOG_DEBUG, "CCollectThread::%s, [%ld]\n", __FUNCTION__, ((seconds * 1000) % FLUSH_ACTION_INTERVAL));
    tm_action_.SetInterval(FLUSH_ACTION_INTERVAL - ((seconds * 1000) % FLUSH_ACTION_INTERVAL));
    tm_action_.Start();
    //CTimerManager::Dump();
}
void CCollectThread::DoFlushError() {
    BS_XLOG(XLOG_DEBUG, "CCollectThread::%s\n", __FUNCTION__);
    time_t now = time(NULL);

    boost::unordered_map<SErrorInfo, int>::iterator itr = map_error_.begin();
    for (; itr != map_error_.end(); ++itr) {
		SErrorInfo info = itr->first;
        info.timestamp = now;
        info.count = itr->second;
        CMongoThreadGroup::GetInstance()->OnError(info);
    }
    map_error_.clear();
    //CTimerManager::Dump();
}
void CCollectThread::Dump() {
    BS_XLOG(XLOG_DEBUG, "====== CCollectThread::%s =======\n", __FUNCTION__);
    boost::unordered_map<SCurveInfo, int>::iterator itr = map_action_.begin();
    for (; itr != map_action_.end(); ++itr) {
        BS_XLOG(XLOG_DEBUG, " [%s-%s-%s-%s]:[%d]\n", itr->first.projectid.c_str(),itr->first.curveid.c_str(), 
            itr->first.ip.c_str(), itr->first.page.c_str(), itr->second);
    }
}
}
}
