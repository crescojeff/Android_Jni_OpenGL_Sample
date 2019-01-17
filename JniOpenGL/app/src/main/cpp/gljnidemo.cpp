#include <jni.h>

extern "C" JNIEXPORT jboolean JNICALL
Java_com_jeffcreswell_jniopengl_jni_JniHooks_glInit(
        JNIEnv *env,
        jobject /* this */) {
    // todo: init GL renderer
}

extern "C" JNIEXPORT void JNICALL
Java_com_jeffcreswell_jniopengl_jni_JniHooks_glStepFrame(
        JNIEnv *env,
jobject /* this */) {
// todo: render the next frame
}
