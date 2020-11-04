#include <jni.h>
#include <android/log.h>
#include <thread>
#include <future>
#include <tools/LoggingCallback.h>
#include <include/NativeBypass/bypass.h>
#include "../../../../src/DiskLoader.h"

JavaVM *_vm;

static jint Java_loadFromStorage(
        JNIEnv *env,
        jclass clazz,
        jstring name) {

}

static jint Java_load(
        JNIEnv *env,
        jclass clazz,
        jstring jpath) {
    jboolean isCopy;
    const char *str = env->GetStringUTFChars(jpath, &isCopy);
    std::string path(str);
    return DiskLoader::load(path,_vm);

    return 1;
}
static jlong Java_getAddr(
        JNIEnv *env,
        jclass clazz,
        jstring library,
        jstring symbol) {
    jboolean isCopy;
    const char *lib_cstr = env->GetStringUTFChars(library, &isCopy);
    std::string lib(lib_cstr);
    const char *sym_cstr = env->GetStringUTFChars(symbol, &isCopy);
    std::string sym(sym_cstr);
    return (long) ChickenHook::NativeBypass::Resolve::ResolveSymbol(lib,sym);

    return 1;
}


static const JNINativeMethod gMethods[] = {
        {"getAddr",            "(Ljava/lang/String;Ljava/lang/String;)J", (void *) Java_getAddr},
        {"load",            "(Ljava/lang/String;)I", (void *) Java_load},
        {"loadFromStorage", "(Ljava/lang/String;)I", (void *) Java_loadFromStorage}
};
static const char *classPathName = "com/kobil/libloader/NativeInterface";


static int registerNativeMethods(JNIEnv *env, const char *className,
                                 JNINativeMethod *gMethods, int numMethods) {
    jclass clazz;
    clazz = env->FindClass(className);
    if (clazz == nullptr) {
        __android_log_print(ANDROID_LOG_DEBUG, "registerNativeMethods",
                            "Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        __android_log_print(ANDROID_LOG_DEBUG, "registerNativeMethods",
                            "Native registration unable to register natives...");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}


jint JNI_OnLoad(JavaVM *vm, void * /*reserved*/) {
    _vm = vm;
    JNIEnv *env = nullptr;
    if (vm->GetEnv((void **) (&env), JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }


    if (!registerNativeMethods(env, classPathName,
                               (JNINativeMethod *) gMethods,
                               sizeof(gMethods) / sizeof(gMethods[0]))) {
        return -1;
    }

    setLogFunction([](const std::string &str) {
        __android_log_print(ANDROID_LOG_DEBUG, "Interface", "%s", str.c_str());
    });

    ChickenHook::setLogFunction([](const std::string str) {
        __android_log_print(ANDROID_LOG_DEBUG, "Interface", "%s", str.c_str());
    });

    return JNI_VERSION_1_4;
}