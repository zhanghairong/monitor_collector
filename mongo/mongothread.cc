#include "mongothread.h"
#include "mongohelper.h"
#include "configmanager.h"
#include "watchdogthread.h"
#include <bson.h>
#include <ape/loghelper.h>

namespace ape {
namespace monitor {

const int SELF_CHECK_INTERVAL = 3000; //3s

CMongoThread::CMongoThread(ape::common::MsgQueuePrio *queue, int no) : 
    ape::common::MsgTimerThread(queue), id_(no), isalive_(false),
    tmselfcheck_(this, SELF_CHECK_INTERVAL, boost::bind(&CMongoThread::DoSelfCheck, this), ape::common::CThreadTimer::TIMER_CIRCLE) 
{
    funcmap_[E_Db_Connect] = &CMongoThread::DoConnect;
    funcmap_[E_Db_Action] = &CMongoThread::DoAction;
    funcmap_[E_Db_Error] = &CMongoThread::DoError;
}
CMongoThread::~CMongoThread() {
}
void CMongoThread::StartInThread() {
    BS_XLOG(XLOG_DEBUG,"CMongoThread::%s, id[%d]\n", __FUNCTION__, id_);
    tmselfcheck_.Start();
}
void CMongoThread::StopInThread() {
    BS_XLOG(XLOG_DEBUG,"CMongoThread::%s, id[%d]\n", __FUNCTION__, id_);
    tmselfcheck_.Stop();
    boost::unordered_map<std::string, CMongoClient *>::iterator itr;
    for (itr = clients_.begin(); itr != clients_.end(); ++itr) {
        delete itr->second;
    }
    clients_.clear();
}
void CMongoThread::Deal(void *data) {
    SDbMsg *msg = (SDbMsg *)data;
    (this->*(funcmap_[msg->type]))(msg);
    delete msg;
}
void CMongoThread::DoConnect(SDbMsg *data) {
    SConnectDbMsg *msg = (SConnectDbMsg *)data;
    BS_XLOG(XLOG_TRACE,"CMongoThread::%s, id[%d], addr[%s]\n", __FUNCTION__, id_, msg->addr.c_str());
    boost::unordered_map<std::string, CMongoClient *>::iterator itr = clients_.find(msg->addr);
    if (itr != clients_.end()){
        return;
    }
    CMongoClient *client = new CMongoClient();
    client->Connect(msg->addr);
    clients_[msg->addr] = client;
}
void CMongoThread::DoAction(SDbMsg *data) {
    SActionDbMsg *msg = (SActionDbMsg *)data;
    BS_XLOG(XLOG_DEBUG,"CMongoThread::%s, id[%d], projectid[%s]\n", __FUNCTION__, id_, msg->curveinfo.projectid.c_str());
    CMongoClient *client = GetClient();
    
    ape::monitor::SCurveInfo &info = msg->curveinfo;
    if (info.projectid.length() != 24 || info.curveid.length() != 24) {
        BS_XLOG(XLOG_WARNING,"CMongoThread::%s, invalid curveid[%s], projectid[%s]\n", __FUNCTION__, 
            info.curveid.c_str(), info.projectid.c_str());
        return;
    }
    bson_oid_t oid, cid;
    bson_oid_init_from_time(&oid, info.timestamp);
    bson_oid_init_from_string(&cid, info.curveid.c_str());
    bson_t doc;
    bson_init(&doc);
    BSON_APPEND_OID(&doc, "_id", &oid);
    BSON_APPEND_OID(&doc, "cid", &cid);
    BSON_APPEND_UTF8(&doc, "page", info.page.c_str());
    BSON_APPEND_UTF8(&doc, "ip", info.ip.c_str());
    BSON_APPEND_INT32(&doc, "n", info.count);

    struct tm localtm;
    localtime_r(&(info.timestamp), &localtm);
    
    const std::string &dbprefix = ape::monitor::ConfigManager::GetInstance()->GetActionDbPrefix();
    const std::string &collprefix = ape::monitor::ConfigManager::GetInstance()->GetActionCollectionPrefix();
  
    char szcollection[1024] = {0};
    snprintf(szcollection, 1023, "%s_%04d_%02d.%s_%s_%04d_%02d_%02d", dbprefix.c_str(), localtm.tm_year + 1900, localtm.tm_mon + 1, 
        collprefix.c_str(), info.projectid.c_str(), localtm.tm_year + 1900, localtm.tm_mon + 1, localtm.tm_mday);
    
    if (NULL == client || 0 != client->Insert(szcollection, &doc)) {
        char *str = bson_as_json(&doc, NULL);
        BS_XLOG(XLOG_WARNING,"CMongoThread::%s, insert failed, collection[%s], record[%s]\n", __FUNCTION__, 
            szcollection, str);
        bson_free(str);
        WatchDogThread::GetInstance()->OnActionFail(info);
    }
    
    bson_destroy(&doc);
}
void CMongoThread::DoError(SDbMsg *data) {
    SErrorDbMsg *msg = (SErrorDbMsg *)data;
    ape::monitor::SErrorInfo &info = msg->info;
    BS_XLOG(XLOG_DEBUG,"CMongoThread::%s, id[%d], projectid[%s]\n", __FUNCTION__, id_, info.projectid.c_str());
    CMongoClient *client = GetClient();
    
    if (info.projectid.length() != 24) {
        BS_XLOG(XLOG_WARNING,"CMongoThread::%s, invalid projectid[%s], ip[%s]\n", __FUNCTION__, 
            info.projectid.c_str(), info.ip.c_str());
        return;
    }

    bson_oid_t oid, pid;
    bson_oid_init_from_time(&oid, info.timestamp);
    bson_oid_init_from_string(&pid, info.projectid.c_str());
    bson_t doc;
    bson_init(&doc);
    BSON_APPEND_OID(&doc, "_id", &oid);
    BSON_APPEND_OID(&doc, "pid", &pid);
    BSON_APPEND_INT32(&doc, "code", info.code);
    BSON_APPEND_UTF8(&doc, "ip", info.ip.c_str());
    BSON_APPEND_INT32(&doc, "count", info.count);
    BSON_APPEND_UTF8(&doc, "msg", info.msg.c_str());
	
    struct tm localtm;
    localtime_r(&(info.timestamp), &localtm);
	
    const std::string &dbprefix = ape::monitor::ConfigManager::GetInstance()->GetErrorDbPrefix();
    const std::string &collprefix = ape::monitor::ConfigManager::GetInstance()->GetErrorCollectionPrefix();
  
    char szcollection[1024] = {0};
    snprintf(szcollection, 1023, "%s_%04d_%02d.%s_%s_%04d_%02d_%02d", dbprefix.c_str(), localtm.tm_year + 1900, localtm.tm_mon + 1, 
        collprefix.c_str(), info.projectid.c_str(), localtm.tm_year + 1900, localtm.tm_mon + 1, localtm.tm_mday);

    if (NULL == client || 0 != client->Insert(szcollection, &doc)) {
        char *str = bson_as_json(&doc, NULL);
        BS_XLOG(XLOG_WARNING,"CMongoThread::%s, insert failed, collection[%s], record[%s]\n", __FUNCTION__, 
            szcollection, str);
        bson_free(str);
        WatchDogThread::GetInstance()->OnErrorFail(info);
    }
    
    bson_destroy(&doc);
}
void CMongoThread::DoSelfCheck() {
    //BS_XLOG(XLOG_TRACE,"CMongoThread::%s, id[%d]\n", __FUNCTION__, id_);
    isalive_ = true;
}
void CMongoThread::GetSelfCheck(bool &isalive) {
    isalive = isalive_;
    isalive_ = false;
}
CMongoClient *CMongoThread::GetClient() {
    if (clients_.empty()) {
        return NULL;
    }
    return clients_.begin()->second;
}

/************************************************************************************/
/***********************  CMongoThreadGroup  ****************************************/
/************************************************************************************/
CMongoThreadGroup *CMongoThreadGroup::sminstance_ = NULL;
CMongoThreadGroup *CMongoThreadGroup::GetInstance() {
    if (sminstance_ == NULL) {
        sminstance_ = new CMongoThreadGroup();
    }
    return sminstance_;
}
void CMongoThreadGroup::Release() {
    if(sminstance_) {
        delete sminstance_;
    }
}
CMongoThreadGroup::CMongoThreadGroup() {
    mongoc_init();
}
CMongoThreadGroup::~CMongoThreadGroup() {
    mongoc_cleanup();
}
void CMongoThreadGroup::Start(int threads) {
    BS_XLOG(XLOG_DEBUG,"CMongoThreadGroup::%s, threads[%d]\n", __FUNCTION__, threads);
    for (int i = 0; i < threads; ++i) {
        CMongoThread *thread = new CMongoThread(&queue_, i);
        threads_.push_back(thread);
        thread->Start();
    }
}
void CMongoThreadGroup::Stop() {
    BS_XLOG(XLOG_DEBUG,"CMongoThreadGroup::%s\n", __FUNCTION__);
    std::vector<CMongoThread *>::iterator itr;
    for (itr = threads_.begin(); itr != threads_.end(); ++itr) {
        (*itr)->Stop();
    }
    sleep(1);
    for (itr = threads_.begin(); itr != threads_.end(); ++itr) {
        delete *itr;
    }
    threads_.clear();
}
void CMongoThreadGroup::OnConnect(const std::string &addr) {
    BS_XLOG(XLOG_TRACE,"CMongoThread::%s, addr[%s]\n", __FUNCTION__, addr.c_str());
    std::vector<CMongoThread *>::iterator itr;
    for (itr = threads_.begin(); itr != threads_.end(); ++itr) {
        SConnectDbMsg *msg = new SConnectDbMsg();
        msg->addr = addr;
        (*itr)->PutQ(msg);
    }
}
void CMongoThreadGroup::OnAction(const SCurveInfo &info) {
    BS_XLOG(XLOG_DEBUG,"CMongoThreadGroup::%s, projectid[%s]\n", __FUNCTION__, info.projectid.c_str());
    SActionDbMsg *msg = new SActionDbMsg;
    msg->curveinfo = info;
    if (false == queue_.PutQ(msg, 3000)) {
        delete msg;
        WatchDogThread::GetInstance()->OnActionFail(info);
    }
}
void CMongoThreadGroup::OnError(const SErrorInfo &info) {
    SErrorDbMsg *msg = new SErrorDbMsg;
    msg->info = info;
    if (false == queue_.PutQ(msg, 3000)) {
        delete msg;
        WatchDogThread::GetInstance()->OnErrorFail(info);
    }
}
}
}
