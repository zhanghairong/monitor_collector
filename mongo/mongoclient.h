#ifndef _APE_COMMON_MONGO_CLIENT_H_
#define _APE_COMMON_MONGO_CLIENT_H_
#include <mongoc.h>
#include <boost/unordered_map.hpp>
#include <bson.h>

namespace ape {
namespace monitor {
class CMongoClient {
 public:
    CMongoClient();
    ~CMongoClient();
    int Connect(const std::string &addr);
    int CreateIndex(const std::string &name, const bson_t *keys);
    int DropIndex(const std::string &name, const std::string &index);
    int Query(std::vector<bson_t *> *result, const std::string &name, const bson_t *query, 
        const bson_t *fields = NULL, uint32_t skip = 0, uint32_t limit = 0, uint32_t batch_size = 0);
    int Insert(const std::string &name, const bson_t *document);
    int Update(const std::string &name, const bson_t *selector, const bson_t *update, bool multi = false);
 private:
    int ConnectInner();
    mongoc_collection_t *GetCollection(const std::string &name);
 private:
    std::string addr_;
    mongoc_client_t *client_;
    boost::unordered_map<std::string, mongoc_collection_t *> collections_; //key is database.collection
};

}
}
#endif
