#include <GLES3/gl32.h>
#include <GLES3/gl3ext.h>
#include <jni.h>
#include <android/log.h>
#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale
#include <cstdio>
#include <cstdlib>
#include <cmath>

#define  LOG_TAG    "gles3jnidemo"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

/**
 * The OpenGL program ID for our shader
 */
GLuint gProgram;
/**
 * Vertex Buffer Object we'll use to upload data to the GPU
 */
GLuint gVboID;
/**
 * Vertex Array Objects allow us to maintain bindings between vertex data
 * in the VBO and shader program attributes.  That way we can modify vertex
 * data simply by switching the bound VAO.
 */
GLuint gVaoID;
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
 * Simply sets the gl_Position for each vertex to the uploaded vertex data's
 * X,Y coordinates, specifying 0 for Z since we're in 2D at the moment.
 */
const char* gVertexShaderGLES3_noTransform = R"glsl(
    #version 300 es
    in vec2 position;
    void main()
    {
        gl_Position = vec4(position, 0.0, 1.0);
    }
)glsl";
/**
 * Simply sets the outColor output to solid Plum purple
 */
const char* gFragmentShaderGLES3_solidPurple = R"glsl(
        #version 300 es

        out vec4 outColor;

        void main()
        {
            outColor = vec4(0.917647,  0.678431, 0.917647, 1.0);
        }
)glsl";

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
    // our triangle data, with vertices already in device coordinates between -1 and 1
    float vertices[] = {
            0.0f,  0.5f, // Vertex 1 (X, Y)
            0.5f, -0.5f, // Vertex 2 (X, Y)
            -0.5f, -0.5f  // Vertex 3 (X, Y)
    };
    // generate one vertex buffer object and store our handle to it
    glGenBuffers(1,&gVboID);
    // make the VBO we just generated the active array
    glBindBuffer(GL_ARRAY_BUFFER,gVboID);
    // upload our vertex data to the now-active VBO
    // GL_STREAM_DRAW mode is important for any vertex data we expect
    // to be modified client-side and re-uploaded -- 1 draw/upload allows
    // for frame-by-frame rotation
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // generate and bind VAO so that our attibute -> VBO linkages below will
    // be tracked
    glGenVertexArrays(1, &gVaoID);
    glBindVertexArray(gVaoID);

    // now that data has been uploaded to the GPU, we need to tell the
    // GPU what to do with it -- that is where our shaders come in
    gProgram = createProgram(gVertexShaderGLES3_noTransform,gFragmentShaderGLES3_solidPurple);
    if (!gProgram) {
        LOGE("Could not create program.");
        return false;
    }
    // activate our shader program
    glUseProgram(gProgram);
    // fetch the index of the named input, position
    GLint posAttrib = glGetAttribLocation(gProgram, "position");
    checkGlError("glGetAttribLocation");
    LOGI("glGetAttribLocation(\"position\") = %d\n",
         posAttrib);
    // instruct the vertex shader to consider our vertex data stored
    // in input position as having two components per attribute
    // (X,Y coords that will be used to populate the position vec2),
    // that components are floats, that components do not needing normalization to device coords,
    // that stride (space between vertex attribute data) is 0, and that offset (space before
    // vertex attribute data starts) is 0.  This function will also bind
    // the attribute to the VBO currently bound to GL_ARRAY_BUFFER
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    // enable the vertex attribute array
    glEnableVertexAttribArray(posAttrib);

    // set up a viewport with the given width and height dimensions
    glViewport(0, 0, w, h);
    checkGlError("glViewport");
    return true;
}

void renderFrame() {
    // tell the GPU to render our first (and only) 3 vertices
    glDrawArrays(GL_TRIANGLES, 0, 3);
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


