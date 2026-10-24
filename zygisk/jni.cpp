#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <cstring>
#include <fstream>
#include <vector>
#include "zygisk.hpp"

#define LOG_TAG "MAMO_ZYGISK"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

class MamoballModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
        env->GetJavaVM(&this->vm);
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        const char *process = env->GetStringUTFChars(args->nice_name, nullptr);
        
        if (process && strstr(process, "com.alberun.mamoball") != nullptr) {
            LOGI("Mamoball detectado! Carregando DEX e Lib...");
            
            // 1. Carrega a libc++_shared.so
            dlopen("/data/adb/modules/mamoball_macro/zygisk/libc++_shared.so", RTLD_NOW | RTLD_GLOBAL);
            
            // 2. Pega o Context e o ClassLoader do Android
            jclass activity_thread = env->FindClass("android/app/ActivityThread");
            jmethodID current_activity_thread = env->GetStaticMethodID(activity_thread, "currentActivityThread", "()Landroid/app/ActivityThread;");
            jobject at = env->CallStaticObjectMethod(activity_thread, current_activity_thread);
            
            jmethodID get_system_context = env->GetMethodID(activity_thread, "getSystemContext", "()Landroid/content/Context;");
            jobject system_context = env->CallObjectMethod(at, get_system_context);
            
            jclass context_class = env->GetObjectClass(system_context);
            jmethodID get_class_loader = env->GetMethodID(context_class, "getClassLoader", "()Ljava/lang/ClassLoader;");
            jobject class_loader = env->CallObjectMethod(system_context, get_class_loader);

            // 3. Lê o DEX e cria o InMemoryDexClassLoader
            std::ifstream dex_file("/data/adb/modules/mamoball_macro/zygisk/overlay.dex", std::ios::binary);
            std::vector<char> dex_buffer((std::istreambuf_iterator<char>(dex_file)), std::istreambuf_iterator<char>());

            jbyteArray dexBytes = env->NewByteArray(dex_buffer.size());
            env->SetByteArrayRegion(dexBytes, 0, dex_buffer.size(), (const jbyte*)dex_buffer.data());
            
            jclass byte_buffer_class = env->FindClass("java/nio/ByteBuffer");
            jmethodID wrap_method = env->GetStaticMethodID(byte_buffer_class, "wrap", "([B)Ljava/nio/ByteBuffer;");
            jobject byte_buffer = env->CallStaticObjectMethod(byte_buffer_class, wrap_method, dexBytes);

            jclass dex_loader_class = env->FindClass("dalvik/system/InMemoryDexClassLoader");
            jmethodID dex_loader_init = env->GetMethodID(dex_loader_class, "<init>", "(Ljava/nio/ByteBuffer;Ljava/lang/ClassLoader;)V");
            jobject dex_loader = env->NewObject(dex_loader_class, dex_loader_init, byte_buffer, class_loader);

            // 4. Carrega a classe FlorkOverlay do DEX injetado
            jclass flork_class = env->FindClass("com/flork/macro/FlorkOverlay");
            if (flork_class) {
                LOGI("FlorkOverlay encontrada! O botao vai aparecer quando o jogo carregar a Activity.");
                // O botão será instalado pelo Lifecycle do jogo, mas já garantimos que a classe está na memória.
            }
        }
    }

private:
    zygisk::Api *api;
    JNIEnv *env;
    JavaVM *vm;
};

REGISTER_ZYGISK_MODULE(MamoballModule)
