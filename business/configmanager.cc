#include "configmanager.h"
#include "filehelper.h"
#include <ape/loghelper.h>
#include <ape/msgtimerthread.h>
#include <ape/xmlconfigparser.h>

namespace ape {
namespace monitor {

ConfigManager *ConfigManager::sm_instance_ = NULL;
ConfigManager *ConfigManager::GetInstance() {
    if(sm_instance_ == NULL)
        sm_instance_ = new ConfigManager;
    return sm_instance_;
}
int ConfigManager::Init(const std::string &configfile) {
    actiondb_ = "monitor_action";
    errordb_ = "monitor_error";
    action_collectoin_ = "action";
    error_collectoin_ = "error";
    
    ape::common::CXmlConfigParser parser;
    if (0 != parser.ParseFile(configfile)) {
        BS_XLOG(XLOG_FATAL,"parse config[%s] failed, error[%s]\n", configfile.c_str(), parser.GetErrorMessage().c_str());
        return -1;
    }
    action_record_file_ = parser.GetParameter("failedfile/action", "./faileddata/action_record");
    error_record_file_ = parser.GetParameter("failedfile/error", "./faileddata/error_record");

    return 0;
}
}
}
