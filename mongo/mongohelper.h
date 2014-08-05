#ifndef _APE_DATABASE_MONGO_HELPER_H_
#define _APE_DATABASE_MONGO_HELPER_H_
#include <ape/loghelper.h>

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif
static pthread_mutex_t mutex_init = PTHREAD_MUTEX_INITIALIZER;

static void bson_set_oid_host(bson_oid_t *oid) {
    uint8_t *bytes = (uint8_t *)oid;
    static uint8_t digest[16] = {0};
    if (digest[0] == 0) {
        pthread_mutex_lock (&mutex_init);
        bson_md5_t md5;
        static char hostname[HOST_NAME_MAX] = {0};
        gethostname(hostname, sizeof hostname);
        hostname[HOST_NAME_MAX - 1] = '\0';
        //BS_XLOG(XLOG_TRACE, "bson_set_oid_host, hostname[%s]\n", hostname);
        bson_md5_init (&md5);
        bson_md5_append (&md5, (const uint8_t *)hostname, (uint32_t)strlen (hostname));
        bson_md5_finish (&md5, &digest[0]);
        pthread_mutex_unlock(&mutex_init);
    }
    bytes[4] = digest[0];
    bytes[5] = digest[1];
    bytes[6] = digest[2];
}
static void bson_set_pid(bson_oid_t *oid) {
    static uint16_t pid = 0;
    if (pid == 0) {
        pthread_mutex_lock(&mutex_init);
        pid = getpid();
        //BS_XLOG(XLOG_TRACE, "bson_set_pid, pid[%d]\n", pid);
        pid = BSON_UINT16_TO_BE(pid);
        pthread_mutex_unlock(&mutex_init);
    }
    uint8_t *bytes = (uint8_t *)&pid;
    oid->bytes[7] = bytes[0];
    oid->bytes[8] = bytes[1];
}
static void bson_set_inc(bson_oid_t *oid) {
    static uint32_t smcount = 0;
    if (smcount == 0) {
        pthread_mutex_lock (&mutex_init);
        struct timeval tv;
        unsigned int seed[3];
        unsigned int real_seed;
        bson_gettimeofday (&tv, NULL);
        seed[0] = tv.tv_sec;
        seed[1] = tv.tv_usec;
        seed[2] = getpid();
        real_seed = seed[0] ^ seed[1] ^ seed[2];
        smcount = rand_r (&real_seed) & 0x007FFFF0;
        //BS_XLOG(XLOG_TRACE, "bson_set_inc, smcount[%d]\n", smcount);
        pthread_mutex_unlock(&mutex_init);
    }
    uint32_t seq = bson_atomic_int_add(&smcount, 1);
    seq = BSON_UINT32_TO_BE(seq);
    memcpy (&oid->bytes[9], ((uint8_t *)&seq) + 1, 3);
}
static void bson_oid_init_from_time(bson_oid_t *oid, time_t tm) {
    uint32_t t = BSON_UINT32_TO_BE(tm);
    memcpy(&(oid->bytes[0]), &t, 4);
    bson_set_oid_host(oid);
    bson_set_pid(oid);
    bson_set_inc(oid);
}

#endif
