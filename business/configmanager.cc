#include "configmanager.h"
#include "filehelper.h"
#include <ape/loghelper.h>
#include <ape/msgtimerthread.h>

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
    action_record_file_ = "./faileddata/action_record";
    error_record_file_ = "./faileddata/error_record";

    return 0;
}
}
}
