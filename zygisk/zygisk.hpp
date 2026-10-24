#pragma once
#include <jni.h>
#include <sys/types.h>

namespace zygisk {
    struct AppSpecializeArgs {
        jstring nice_name;
        jstring dir;
        jstring fds_to_ignore;
        jintArray rlimits;
        jint uid;
        jint gid;
        jint runtime_flags;
        jint mount_external;
        jstring se_info;
        jint nice_nice;
    };

    struct ServerSpecializeArgs {
        jint uid;
        jint gid;
        jintArray rlimits;
    };

    class Api {
    public:
        virtual void connectCompanion() = 0;
        virtual void setOption(int) = 0;
    };

    class ModuleBase {
    public:
        virtual void onLoad(Api *api, JNIEnv *env) {}
        virtual void preAppSpecialize(AppSpecializeArgs *args) {}
        virtual void postAppSpecialize(const AppSpecializeArgs *args) {}
        virtual void preServerSpecialize(ServerSpecializeArgs *args) {}
        virtual void postServerSpecialize(const ServerSpecializeArgs *args) {}
    };
}

#define REGISTER_ZYGISK_MODULE(cls) \
    extern "C" __attribute__((visibility("default"))) void zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
        static cls instance; instance.onLoad(api, env); \
    }
