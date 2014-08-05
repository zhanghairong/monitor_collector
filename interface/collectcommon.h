#ifndef _APE_MONITOR_COLLECT_COMMON_H_
#define _APE_MONITOR_COLLECT_COMMON_H_
#include <string>
#include <stdio.h>
#include "stringhelper.h"

namespace ape {
namespace monitor {
#define SPLITER '\1'
typedef struct stCurveInfo {
    std::string projectid;
    std::string curveid;
    std::string ip;
    std::string page;
    time_t timestamp;
    int count;
    stCurveInfo():count(0){}
    stCurveInfo(const std::string &pid, const std::string &cid, const std::string &strip, const std::string &pg) : 
        projectid(pid), curveid(cid), ip(strip), page(pg), count(0) {}
    
    bool operator == (const stCurveInfo& k) const {  
        return projectid == k.projectid && ip == k.ip && curveid == k.curveid && page == k.page;  
    }
    friend std::size_t hash_value(const stCurveInfo& k) { 
        size_t seed = 0;  
        boost::hash_combine(seed, boost::hash_value(k.projectid));
        boost::hash_combine(seed, boost::hash_value(k.curveid));
        boost::hash_combine(seed, boost::hash_value(k.ip));
        boost::hash_combine(seed, boost::hash_value(k.page));
        return seed;  
    }
	std::string ToString() const {
		char sznum[64] = {0};
		snprintf(sznum, 63, "%ld%c%d", timestamp, SPLITER, count);
		std::string str = projectid;
		str.append(1, SPLITER).append(curveid).append(1, SPLITER).append(ip).append(1, SPLITER).append(page).append(1, SPLITER).append(sznum);
		return str;
	}
	int InitFromString(const std::string &record)  {
		std::vector<std::string> vec;
		ape::common::split(record, SPLITER, &vec);
		if (vec.size() != 6) {
			return -1;
		}
		projectid = vec[0];
		curveid = vec[1];
		ip = vec[2];
		page = vec[3];
		timestamp = atol(vec[4].c_str());
		count = atol(vec[5].c_str());
		return 0;
	}
}SCurveInfo;
typedef struct stErrorInfo {
    std::string projectid;
    std::string ip;
    std::string msg;
    int code;
    time_t timestamp;
    int count;
    stErrorInfo():code(0){}
    stErrorInfo(const std::string &pid, const std::string &strip, int cd) : 
        projectid(pid), ip(strip), code(cd), count(0) {}
    
    bool operator == (const stErrorInfo& k) const {  
        return projectid == k.projectid && ip == k.ip && code == k.code;
    }
    friend std::size_t hash_value(const stErrorInfo& k) { 
        size_t seed = 0;  
        boost::hash_combine(seed, boost::hash_value(k.projectid));
        boost::hash_combine(seed, boost::hash_value(k.ip));
        boost::hash_combine(seed, boost::hash_value(k.code));
        return seed;  
    }
	std::string ToString() const {
		char sznum[64] = {0};
		snprintf(sznum, 63, "%d%c%ld%c%d", code, SPLITER, timestamp, SPLITER, count);
		std::string str = projectid;
		str.append(1, SPLITER).append(ip).append(1, SPLITER).append(msg).append(1, SPLITER).append(sznum);
		return str;
	}
	int InitFromString(const std::string &record) {
		std::vector<std::string> vec;
		ape::common::split(record, SPLITER, &vec);
		if (vec.size() != 6) {
			return -1;
		}
		projectid = vec[0];
		ip = vec[1];
		msg = vec[2];
		code = atol(vec[3].c_str());
		timestamp = atol(vec[4].c_str());
		count = atol(vec[5].c_str());
		return 0;
	}
}SErrorInfo;

}
}
#endif
