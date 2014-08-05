#include "businesshandle.h"
#include <ape/loghelper.h>
#include <ape/urlcode.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TBufferTransports.h>
#include <boost/shared_ptr.hpp>
#include "collectthread.h"

namespace ape {
namespace monitor {

CBusinessHandle::CBusinessHandle() {
    mapurlfunc_["/stat_action"] = &CBusinessHandle::OnStatAction;
    mapurlfunc_["/error_report"] = &CBusinessHandle::OnErrorReport;
    mapurlfunc_["/test"] = &CBusinessHandle::OnTest;
}
void CBusinessHandle::StartInThread(int threadid, ape::net::CNetService *service, ape::common::CTimerManager *owner) {
    threadid_ = threadid;
    service_ = service;
    timerowner_ = owner;
}
void CBusinessHandle::OnEvent(int threadid, void *event) {
    ape::message::SEvent *e = (ape::message::SEvent *)event;
    BS_XLOG(XLOG_TRACE,"CBusinessHandle::%s, type[%d], threadid[%d]\n", __FUNCTION__, e->id, threadid);
    if (ape::message::HTTP_MESSAGE_EVENT == e->id) {
        DealHttpRequest((ape::message::SHttpMessage *)e);
        return;
    }
    delete e;
}
void CBusinessHandle::DealHttpRequest(ape::message::SHttpMessage *message) {
    message->Dump();
    std::string::size_type pos = message->url.find('?');
    std::string urlpath = pos == std::string::npos ? message->url : message->url.substr(0, pos);
    transform(urlpath.begin(), urlpath.end(), urlpath.begin(), ::tolower);
    boost::unordered_map<std::string, DealUrlFunc>::iterator itr = mapurlfunc_.find(urlpath);
    if (itr == mapurlfunc_.end()) {
        DoSendHttpResponse(message, 404, "{\"return_code\":-10200404, \"return_message\":\"Bad Url\"}");
        return;
    }
    std::string param;
    if (0 == strncasecmp(message->method.c_str(), "get", 3)) {
        if (pos != std::string::npos) {
            param = message->url.substr(pos + 1);
        }
    } else if (0 == strncasecmp(message->method.c_str(), "post", 4)) {
        param = message->body;
    }    
    (this->*(itr->second))(message, param);
}
void CBusinessHandle::DoSendHttpResponse(ape::message::SHttpMessage *request, int code, const std::string &body) {
    ape::message::SHttpMessage *response = new ape::message::SHttpMessage;
    response->SetReply(code);
    response->requestno = request->requestno;
    response->keepalive = request->keepalive;
    response->httpversion = request->httpversion;
    response->body = body;
    service_->DoSendBack(request->connid, response);
    delete request;
}
void CBusinessHandle::OnTest(ape::message::SHttpMessage *message, const std::string &param) {
    DoSendHttpResponse(message, 200, "{\"return_code\":0, \"return_message\":\"test success\"}");
}
void CBusinessHandle::OnStatAction(ape::message::SHttpMessage *message, const std::string &param) {
    std::string strprojectid;
    std::string ip;
    std::vector<std::string> vecaction;
    DecodeActionParam(param.c_str(), param.length(), &strprojectid, &ip, &vecaction);
    if (strprojectid.empty() || ip.empty() || vecaction.empty()) {
        DoSendHttpResponse(message, 400, "{\"return_code\":-10200400, \"return_message\":\"Bad Parameter\"}");
        return;
    }
    CCollectThread::GetInstance()->OnStatAction(strprojectid, ip, vecaction);
    DoSendHttpResponse(message, 200, "{\"return_code\":0, \"return_message\":\"success\"}");
}
void CBusinessHandle::OnErrorReport(ape::message::SHttpMessage *message, const std::string &param) {
    SErrorInfo info;
    std::vector<std::string> vecaction;
    DecodeErrorReportParam(param.c_str(), param.length(), &info);
    if (info.projectid.empty() || info.ip.empty() || info.code == 0) {
        DoSendHttpResponse(message, 400, "{\"return_code\":-10200400, \"return_message\":\"Bad Parameter\"}");
        return;
    }
    CCollectThread::GetInstance()->OnErrorReport(info);
    DoSendHttpResponse(message, 200, "{\"return_code\":0, \"return_message\":\"success\"}");
}
void CBusinessHandle::DecodeActionParam(const char *str, int len, std::string *projectid, std::string *ip, std::vector<std::string> *actions) {
    const char *end = str + len;
    const char *begin = str;
    const char *equalchar = begin, *andchar = begin;
    for (const char *p = begin; p < end; ++p) {
        if (*p == '=') {
            equalchar = p;
        } else if(*p == '&') {
            andchar = p;
        } else if(p == end - 1) {
            andchar = end;
        }

        if ( equalchar > begin && andchar >= equalchar + 1) {
            std::string key = URLDecoder::decode(begin, equalchar - begin);
            if (key.length() == 9 && 0 == strncasecmp(key.c_str(), "projectid", 9)) {
                projectid->assign(equalchar + 1, andchar - equalchar -1);
            } else if (key.length() == 8 && 0 == strncasecmp(key.c_str(), "action[]", 8)) {
                actions->push_back(URLDecoder::decode(equalchar + 1, andchar - equalchar -1));
            } else if (key.length() == 2 && 0 == strncasecmp(key.c_str(), "ip", 2)) {
                ip->assign(URLDecoder::decode(equalchar + 1, andchar - equalchar -1));
            }
            begin = andchar + 1;
        }
    }
}
void CBusinessHandle::DecodeErrorReportParam(const char *str, int len, SErrorInfo *info) {
    const char *end = str + len;
    const char *begin = str;
    const char *equalchar = begin, *andchar = begin;
    for (const char *p = begin; p < end; ++p) {
        if (*p == '=') {
            equalchar = p;
        } else if(*p == '&') {
            andchar = p;
        } else if(p == end - 1) {
            andchar = end;
        }

        if ( equalchar > begin && andchar >= equalchar + 1) {
            if (0 == strncasecmp(begin, "projectid", 9)) {
                info->projectid.assign(equalchar + 1, andchar - equalchar -1);
            } else if (0 == strncasecmp(begin, "ip", 2)) {
                info->ip.assign(URLDecoder::decode(equalchar + 1, andchar - equalchar -1));
            } else if (0 == strncasecmp(begin, "msg", 2)) {
                info->msg.assign(URLDecoder::decode(equalchar + 1, andchar - equalchar -1));
            } else if (0 == strncasecmp(begin, "code", 2)) {
                info->code = atoi(equalchar + 1);
            }
            begin = andchar + 1;
        }
    }
}
void CBusinessHandle::Dump() {
}
}
}
