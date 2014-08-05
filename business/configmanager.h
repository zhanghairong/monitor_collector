#ifndef _APE_MONITOR_CONFIG_MANAGER_H_
#define _APE_MONITOR_CONFIG_MANAGER_H_
#include <string>

namespace ape {
namespace monitor {
class ConfigManager {
 public:
    static ConfigManager *GetInstance();
    int Init(const std::string &configfile);
    const std::string &GetActionDbPrefix() const {return actiondb_;}
    const std::string &GetActionCollectionPrefix() const {return action_collectoin_;}
    const std::string &GetErrorDbPrefix() const {return errordb_;}
    const std::string &GetErrorCollectionPrefix() const {return error_collectoin_;}
    const std::string &GetActionRecordFile() const {return action_record_file_;}
    const std::string &GetErrorRecordFile() const {return error_record_file_;}
 private:
    ConfigManager(){}
    static ConfigManager *sm_instance_;
    std::string actiondb_;
    std::string errordb_;
    std::string action_collectoin_;
    std::string error_collectoin_;

    std::string action_record_file_;
    std::string error_record_file_;
};
}
}
#endif
