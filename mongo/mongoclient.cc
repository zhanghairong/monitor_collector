#include "mongoclient.h"
#include <ape/loghelper.h>

namespace ape {
namespace monitor {
CMongoClient::CMongoClient() : client_(NULL) {
}
CMongoClient::~CMongoClient() {
    boost::unordered_map<std::string, mongoc_collection_t *>::iterator itr;
    for (itr = collections_.begin(); itr != collections_.end(); ++itr) {
        mongoc_collection_destroy(itr->second);
    }
    collections_.clear();
    if (client_) {
        mongoc_client_destroy(client_);
        client_ = NULL;
    }
}
int CMongoClient::Connect(const std::string &addr) {
    addr_ = addr;
    return ConnectInner();
}
int CMongoClient::ConnectInner() {
    client_ = mongoc_client_new(addr_.c_str());
    if (!client_) {
        BS_XLOG(XLOG_WARNING, "CMongoClient::%s, Failed to connect addr[%s]\n", __FUNCTION__, addr_.c_str());
        return -1;
    }
    return 0;
}
int CMongoClient::CreateIndex(const std::string &name, const bson_t *keys) {
    mongoc_collection_t *collection = GetCollection(name);
    if (collection == NULL) {
        BS_XLOG(XLOG_WARNING, "CMongoClient::%s, get collection failed, addr[%s], name[%s]\n", 
            __FUNCTION__, addr_.c_str(), name.c_str());
        return -1;
    }
    bson_error_t error;
    if (!mongoc_collection_create_index(collection, keys, NULL, &error)) {
        char *str = bson_as_json(keys, NULL);
        BS_XLOG(XLOG_WARNING, "CMongoClient::%s, Create Index Failed, addr[%s], name[%s], keys[%s], error[%s]\n", 
            __FUNCTION__, addr_.c_str(), name.c_str(), str, error.message);
        bson_free (str);
        return -1;
    }
    return 0;
}
int CMongoClient::DropIndex(const std::string &name, const std::string &index) {
    BS_XLOG(XLOG_DEBUG, "CMongoClient::%s, addr[%s], name[%s], index[%s]\n", __FUNCTION__, addr_.c_str(), name.c_str(), index.c_str());
    mongoc_collection_t *collection = GetCollection(name);
    if (collection == NULL) {
        BS_XLOG(XLOG_WARNING, "CMongoClient::%s, get collection failed, addr[%s], name[%s]\n", 
            __FUNCTION__, addr_.c_str(), name.c_str());
        return -1;
    }
    bson_error_t error;
    if (!mongoc_collection_drop_index(collection, index.c_str(), &error)) {
        BS_XLOG(XLOG_WARNING, "CMongoClient::%s, Drop Index Failed, addr[%s], name[%s], index[%s], error[%s]\n", 
            __FUNCTION__, addr_.c_str(), name.c_str(), index.c_str(), error.message);
        return -1;
    }
    return 0;
}
int CMongoClient::Query(std::vector<bson_t *> *result, const std::string &name, const bson_t *query, 
        const bson_t *fields, uint32_t skip, uint32_t limit, uint32_t batch_size) {
    char *str = bson_as_json(query, NULL);    
    BS_XLOG(XLOG_DEBUG, "CMongoClient::%s, addr[%s], name[%s], query[%s]\n", 
        __FUNCTION__, addr_.c_str(), name.c_str(), str);
    
    mongoc_collection_t *collection = GetCollection(name);
    if (collection == NULL) {
        BS_XLOG(XLOG_WARNING, "CMongoClient::%s, get collection failed, addr[%s], name[%s]\n", 
            __FUNCTION__, addr_.c_str(), name.c_str());
        bson_free(str);
        return -1;
    }
    
    mongoc_cursor_t *cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, skip, limit, batch_size, query, fields, NULL);
    if (cursor == NULL) {
        BS_XLOG(XLOG_WARNING, "CMongoClient::%s, find failed, addr[%s], name[%s], query[%s]\n", 
            __FUNCTION__, addr_.c_str(), name.c_str(), str);
        bson_free(str);
        return -1;
    }
    bson_error_t error;
    const bson_t *doc;
    while(mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &doc)) {
        result->push_back(bson_copy(doc));
    }
    if (mongoc_cursor_error(cursor, &error)) {
        BS_XLOG(XLOG_WARNING, "CMongoClient::%s, find failed, addr[%s], name[%s], query[%s], error[%s]\n", 
            __FUNCTION__, addr_.c_str(), name.c_str(), str, error.message);
        bson_free(str);
        mongoc_cursor_destroy(cursor);
        return -1;
    }
    bson_free(str);
    mongoc_cursor_destroy(cursor);
    return 0;
}
int CMongoClient::Insert(const std::string &name, const bson_t *document) {
    BS_XLOG(XLOG_DEBUG, "CMongoClient::%s, collection[%s]\n", __FUNCTION__, name.c_str());
    mongoc_collection_t *collection = GetCollection(name);
    if (collection == NULL) {
        BS_XLOG(XLOG_WARNING, "CMongoClient::%s, get collection failed, addr[%s], name[%s]\n", 
            __FUNCTION__, addr_.c_str(), name.c_str());
        return -1;
    }
    bson_error_t error;
    if (!mongoc_collection_insert(collection, MONGOC_INSERT_CONTINUE_ON_ERROR, document, NULL, &error)) {
        /*
		char *str = bson_as_json(document, NULL);
        BS_XLOG(XLOG_WARNING, "CMongoClient::%s, Insert Failed, addr[%s], name[%s], document[%s], error[%s]\n", 
            __FUNCTION__, addr_.c_str(), name.c_str(), str, error.message);
        bson_free (str);*/
        return -1;
    }
    return 0;
}
int CMongoClient::Update(const std::string &name, const bson_t *selector, const bson_t *update, bool multi) {
    mongoc_collection_t *collection = GetCollection(name);
    if (collection == NULL) {
        BS_XLOG(XLOG_WARNING, "CMongoClient::%s, get collection failed, addr[%s], name[%s]\n", 
            __FUNCTION__, addr_.c_str(), name.c_str());
        return -1;
    }
    bson_error_t error;
    mongoc_update_flags_t flags = multi ? MONGOC_UPDATE_MULTI_UPDATE : MONGOC_UPDATE_NONE;
    if (!mongoc_collection_update(collection, flags, selector, update, NULL, &error)) {
        char *strq = bson_as_json(selector, NULL);
        char *stru = bson_as_json(update, NULL);
        BS_XLOG(XLOG_WARNING, "CMongoClient::%s, Insert Failed, addr[%s], name[%s], selector[%s], updator[%s], error[%s]\n", 
            __FUNCTION__, addr_.c_str(), name.c_str(), strq, stru, error.message);
        bson_free (strq);
        bson_free (stru);
        return -1;
    }
    return 0;
}
mongoc_collection_t *CMongoClient::GetCollection(const std::string &name) {
    if (client_ == NULL) {
        BS_XLOG(XLOG_WARNING, "CMongoClient::%s, no client to use, addr[%s]\n", __FUNCTION__, addr_.c_str());
        return NULL;
    }
    boost::unordered_map<std::string, mongoc_collection_t *>::iterator itr = collections_.find(name);
    if (itr != collections_.end()) {
        return itr->second;
    }

    std::string::size_type pos = name.find(".");
    if (pos == std::string::npos || pos == 0 || pos == name.length() - 1) {
        BS_XLOG(XLOG_WARNING, "CMongoClient::%s, bad collectoin name[%s]\n", __FUNCTION__, name.c_str());
        return NULL;
    }
    mongoc_collection_t *collection = mongoc_client_get_collection (client_, name.substr(0, pos).c_str(), name.substr(pos + 1).c_str());
    collections_[name] = collection;

    return collection;
}
}
}