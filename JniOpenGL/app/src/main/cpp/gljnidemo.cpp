#include <jni.h>
#include <android/log.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>

#define  LOG_TAG    "gljnidemo"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

/**
 * The OpenGL program ID for our shader
 */
GLuint gProgram;
/**
 * Handle to the vec4 of our vertex shader that tracks current position
 */
GLuint gvPositionHandle;
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
 * Basic vertex shader to use our triangle data to create anchor points, which
 * will be modified by the fragment shader later in the pipeline
 */
auto gVertexShader =
        "attribute vec4 vPosition;\n"
        "void main() {\n"
        /*
         * since vPosition is used as the location to send
         * triangle vertex data, we can draw the triangle by
         * setting the current gl_Position with vPosition
         */
        "  gl_Position = vPosition;\n"
        "}\n";

/**
 * Borrowed from The Book of Shaders [https://thebookofshaders.com/08/],
 * this fragment shader is responsible for performing the actual rotation of
 * space and translation of model coordinates
 */
auto gFragmentShader =
        "precision mediump float;\n"
        "#define PI 3.14159265359"
        "uniform vec2 u_resolution;\n"
        "uniform float u_time;\n"
        /*
         * creates a rotation matrix for the given angle
         */
        "mat2 rotate2d(float _angle){\n"
        "   return mat2(cos(_angle),-sin(_angle),\n"
        "        sin(_angle),cos(_angle));\n"
        "}\n"
        "void main() {\n"
        "    vec2 st = gl_FragCoord.xy/u_resolution.xy;\n"
        "    vec3 color = vec3(0.0);\n"
        "\n"
        // move space from the center to the vec2(0.0)
        "    st -= vec2(0.5);\n"
        // rotate the space with an angle derived from current time
        "    st = rotate2d( sin(u_time)*PI ) * st;\n"
        // move it back to the original place
        "    st += vec2(0.5);"
        /*static opaque green color*/
        //"  gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
        // Add the shape on the foreground
        "    color += vec3(cross(st,0.4));\n"
        "\n"
        "    gl_FragColor = vec4(color,1.0);"
        "}\n";

/**
 * Compiles shader source code and returns a non-zero shader ID if successful
 * @param shaderType specifies the type of shader (e.g. vertex or fragment) to compile
 * @param pSource the raw shader source code string to be compiled
 * @return non-zero shader ID if successful, else zero
 */
GLuint loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGE("Could not compile shader %d:\n%s\n",
                         shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

/**
 * Links the given vertex and fragment shaders together into a single shader program
 * @param pVertexSource raw source code for vertex shader
 * @param pFragmentSource raw source code for fragment shader
 * @return non-zero shader program ID if link is successful, else zero.
 */
GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        checkGlError("glAttachShader");
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOGE("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
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
    gProgram = createProgram(gVertexShader, gFragmentShader);
    if (!gProgram) {
        LOGE("Could not create program.");
        return false;
    }
    gvPositionHandle = glGetAttribLocation(gProgram, "vPosition");
    checkGlError("glGetAttribLocation");
    LOGI("glGetAttribLocation(\"vPosition\") = %d\n",
         gvPositionHandle);

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

    // installs the compiled/linked shader program into the rendering state,
    // activating it
    glUseProgram(gProgram);
    checkGlError("glUseProgram");

    // tell OpenGL how to use our vertex data for rendering
    // in this case, the main point of interest is the size parameter, which
    // tells OpenGL that our data has two components (x,y coordinates) per attribute
    glVertexAttribPointer(gvPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, gTriangleVertices);
    checkGlError("glVertexAttribPointer");
    // specify that we want our triangle data array to be actively used for rendering calls
    glEnableVertexAttribArray(gvPositionHandle);
    checkGlError("glEnableVertexAttribArray");
    // render the triangle data array, as three attributes (with two components each)
    glDrawArrays(GL_TRIANGLES, 0, 3);
    checkGlError("glDrawArrays");
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
