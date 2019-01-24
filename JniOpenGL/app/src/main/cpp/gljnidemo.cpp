#include <jni.h>
#include <android/log.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
/* // GLM would allow for GLSL-like math and structs in GL, might be handy but is not provided as part of NDK
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
*/
#include <cstdio>
#include <cstdlib>
#include <cmath>

#define  LOG_TAG    "gljnidemo"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

/**
 * Our triangle vertex data, given as a float array with six elements representing
 * X and Y coordinates of three vertices.
 * This will be parsed by OpenGL as an array of three vertex attributes with two
 * components (x,y coordinates) each
 */
const GLfloat gTriangleVertices[] = { 0.0f, 0.5f, -0.5f, -0.5f,
                                      0.5f, -0.5f };

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error = glGetError()) {
        LOGI("after %s() glError (0x%x)\n", op, error);
    }
}

/**
 * Configures and creates the OpenGL viewport based on the dimensions of the
 * window from the underlying platform's window system in which OpenGL content will render.
 * Also creates the shader program for use in renderFrame()
 * @param w the width of the display window
 * @param h the height of the display window
 * @return true if the shader program was created successfully and the viewport was set, false otherwise
 */
bool setupGraphics(int w, int h) {
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    LOGI("setupGraphics(%d, %d)", w, h);

    glViewport(0, 0, w, h);
    checkGlError("glViewport");
    return true;
}

void renderFrame() {
    // establish the background clear-color
    static float backgroundColor = 0.0f;
    glClearColor(backgroundColor, backgroundColor, backgroundColor, 1.0f);
    checkGlError("glClearColor");
    // tell OpenGL to use our clear-color
    glClear( GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");

    // todo: borrowed from stackoverflow question about gles 1.x
    // triangle rotation.  Does not render anything as-is, so likely
    // needs some glBegin...glEnd stuff?  Regressing to 1.x seems like
    // it won't actually be simpler than compiling GLM to work with 2.x
    // and shaders seamlessly.

    GLfloat vertices[] = {
            0,0,
            50,100,
            100,0
    };
    GLint colors[] = {
            255, 0, 0, 25,
            0, 255, 0, 255,
            0, 0, 255, 255
    };

    GLint middle[] = {50,50};
    GLint origin[] = {0,0};

    // Rotate the triangle data
    glPushMatrix();
    glTranslatef(50, 50, 0);
    glRotatef(45, 0, 0, 1.0);
    glTranslatef(-50, -50, 0);

    // draw the triangle using rotated data
    glLineWidth(2);
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
    glColor4ub(0, 0, 255, 255);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);

    // Revert rotation, we only want triangle to rotate
    glPopMatrix();

}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_jeffcreswell_jniopengl_jni_JniHooks_glInit(
        JNIEnv *env,
        jobject thiz,
        jint width,
        jint height) {

    return setupGraphics(width, height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_jeffcreswell_jniopengl_jni_JniHooks_glStepFrame(
        JNIEnv *env,
        jobject thiz) {
    renderFrame();
}
