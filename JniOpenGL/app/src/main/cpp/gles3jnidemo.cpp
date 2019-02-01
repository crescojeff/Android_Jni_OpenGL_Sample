#include <GLES3/gl32.h>
#include <GLES3/gl3ext.h>
#include <jni.h>
#include <android/log.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale
#include "glm/gtc/type_ptr.hpp"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <chrono>

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
/**
 * Will hold the time when we set up our GL context, to be used for mods over time
 * when rendering
 */
auto t_start = std::chrono::high_resolution_clock::now();
/**
 * ID for the active fragment shader
 */
GLuint gFragmentShaderID;
/**
 * ID for the active vertex shader
 */
GLuint gVertexShaderID;

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
 * Sets the gl_Position for each vertex to the uploaded vertex data's
 * X,Y coordinates, specifying 0 for Z since we're in 2D at the moment.
 * A transformation matrix is applied via the uniform u_transformationMat,
 * set by the client
 */
const char* gVertexShaderSource = R"glsl(#version 300 es
    in vec2 position;
    uniform mat4 u_transformationMat;
    void main()
    {
        gl_Position = u_transformationMat * vec4(position, 0.0, 1.0);
    }
)glsl";
/**
 * Sets the outColor output based on uniform u_triangleColor, set by client
 */
const char* gFragmentShaderSource = R"glsl(#version 300 es
        precision mediump float;
        uniform vec3 u_triangleColor;
        out vec4 outColor;
        void main()
        {
            outColor = vec4(u_triangleColor, 1.0);
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
    gVertexShaderID = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!gVertexShaderID) {
        return 0;
    }

    gFragmentShaderID = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!gFragmentShaderID) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, gVertexShaderID);
        checkGlError("glAttachShader");
        glAttachShader(program, gFragmentShaderID);
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
    // GL_STATIC_DRAW mode is ideal for cases where we will be
    // rendering uploaded data more often than uploading new data
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // generate and bind VAO so that our attibute -> VBO linkages below will
    // be tracked
    glGenVertexArrays(1, &gVaoID);
    glBindVertexArray(gVaoID);

    // now that data has been uploaded to the GPU, we need to tell the
    // GPU what to do with it -- that is where our shaders come in
    gProgram = createProgram(gVertexShaderSource,gFragmentShaderSource);
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
    t_start = std::chrono::high_resolution_clock::now();
    return true;
}

/**
 * Deletes all persistent resources once we're finished rendering
 */
 /* EGL context destruction takes care of this
void teardownGraphics(){
    glDeleteProgram(gProgram);
    glDeleteShader(gFragmentShaderID);
    glDeleteShader(gVertexShaderID);
    glDeleteBuffers(1, &gVboID);
    glDeleteVertexArrays(1, &gVaoID);
}
*/

void renderFrame() {
    // Clear the screen to black (bonus content: comment this to get a neat psychedlic
    // paint effect as the triangle rotates)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // triangle rave hyyype!
    GLint uniColor = glGetUniformLocation(gProgram, "u_triangleColor");
    auto t_now = std::chrono::high_resolution_clock::now();
    float elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
    glUniform3f(uniColor, (sin(elapsedTime) + 1.0f) / 2.0f, 0.3f, (cos(elapsedTime) + 1.0f) / 2.0f);

    // rotation!
    // init to identity matrix
    glm::mat4 rotationMat = glm::mat4(1.0f);
    // convert to rotation matrix with theta of 180 degrees
    // counterclockwise (positive 1 in Z) over the Z axis
    // todo: for some reason the result is not exactly what I expected --
    // it looks like the axis of rotation is translating over time,
    // and the vertices deform slightly over the course of rotation.
    // A single rotation by 90 degrees shows correct rotation, but
    // the triangle is no longer equilateral.
    rotationMat = glm::rotate(
            rotationMat,
            elapsedTime * glm::radians(180.0f),
            glm::vec3(0.0f, 0.0f, 1.0f)
            );
    /* test vector works as expected, so the rotation math is working as expected
    glm::vec4 simpleRotatedVector = rotationMat * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    LOGI("simple rotated vector was 1.0,0.0,0.0,1.0 before rotation and after is: %f, %f, %f\n", simpleRotatedVector.x, simpleRotatedVector.y, simpleRotatedVector.z);
    */
    /* hmm, nope this results in rotating the whole model around the bottom left corner.
     * could be that since we're starting with device coordinates the translation between world origin and
     * model origin is unneeded?  Interesting!
     glm::mat4 completeTransformationMat = glm::mat4(1.0f);
     glm::mat4 translationToGlOriginMat = glm::mat4(1.0f);
     glm::mat4 translationToModelOriginMat = glm::mat4(1.0f);
     // since the device coordinates will consider 0,0 to be
     // 1 unit in from the left and 1 unit up from the bottom,
     // we need to translate the model as if 0,0 were the bottom left
     // corner
     translationToGlOriginMat = glm::translate(translationToGlOriginMat,glm::vec3(-1.0f,-1.0f,0.0f));
     translationToModelOriginMat = glm::translate(translationToModelOriginMat,glm::vec3(1.0f,1.0f,0.0f));
     completeTransformationMat = translationToGlOriginMat * rotationMat * translationToModelOriginMat;
    */
    // grab a handle to our transformation matrix uniform in the vertex shader
    GLint u_transformationMat = glGetUniformLocation(gProgram, "u_transformationMat");

    // apply rotationMat to our vertex vector by uploading it to GPU in u_transformationMat
    // GLM's value_ptr function will convert out mat4 client side into raw float array
    // of length 16 to accommodate the 4x4 elements.
    glUniformMatrix4fv(u_transformationMat, 1, GL_FALSE, glm::value_ptr(rotationMat));

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

/* EGL takes care of this
extern "C" JNIEXPORT void JNICALL
Java_com_jeffcreswell_jniopengl_jni_JniHooks_glDeinit(
        JNIEnv *env,
        jobject thiz) {
    teardownGraphics();
}
*/

extern "C" JNIEXPORT void JNICALL
Java_com_jeffcreswell_jniopengl_jni_JniHooks_glStepFrame(
        JNIEnv *env,
        jobject thiz) {
    renderFrame();
}


