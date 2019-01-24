#include <jni.h>
#include <android/log.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
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
        "uniform mat4 rotationMatrix;"
        "void main() {\n"
        /*
         * since vPosition is used as the location to send
         * triangle vertex data, we can draw the triangle by
         * setting the current gl_Position with vPosition,
         * and rotate it by setting it to vPosition vector multipled by
         * our rotation matrix
         */
        //"  gl_Position = rotationMatrix * vPosition;\n"
        //"  gl_Position = vPosition;\n"

            // already centered on origin in local coordinate system, so we don't need to translate
        /*
        // translate tri verts to origin
        "mat4 translationMat = mat4(\n"
        "1.0,0.0,0.0,0.5,\n"
        "0.0,1.0,0.0,0.5,\n"
        "0.0,0.0,1.0,0.5,\n"
        "0.0,0.0,0.0,1.0\n"
        ");\n"
        "gl_Position = translationMat * vPosition;\n"
        */
        // hey!  This kind of rotates the triangle... the rotation is off,
        // probably because we need to translate the center of our triangle
        // to the origin of the coordinate space, but I can't figure out how
        // to either communicate the resultant vertex data out to renderFrame()
        // or perform the rotation in renderFrame to be sent into this shader
        // such that the rotations stack and we get the desired spinning shape.
        "   float angle = 180.0;\n"
        "   mat4 rotationMat = mat4(\n"
        "       cos(angle),-sin(angle),0,0,\n"
        "       sin(angle),cos(angle),0,0,\n"
        "       0,0,1,0,\n"
        "       0,0,0,1\n"
        "   );"
            // rotate the local coordinate space
            "gl_Position = rotationMat * vPosition;\n"
        "}\n";

/**
 * Borrowed from The Book of Shaders [https://thebookofshaders.com/08/],
 * this fragment shader is responsible for performing the actual rotation of
 * space and translation of model coordinates
 */
 /*
auto gFragmentShader =
        "precision mediump float;\n"
        "#define PI 3.14159265359"
        "uniform vec2 u_resolution;\n"
        "uniform float u_time;\n"

        //creates a rotation matrix for the given angle
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
        //static opaque green color
        //"  gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
        // Add the shape on the foreground
        "    color += vec3(cross(st,0.4));\n"
        "\n"
        "    gl_FragColor = vec4(color,1.0);"
        "}\n";
        */
 auto gFragmentShader =
         "precision mediump float;\n"
         "void main() {\n"
         //static opaque green color
         "  gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
         "}\n";

int rotationCycle = 0;
 /**
  * Rotates the triangle vertices
  * @param vertexData a float array of six values,
  * every two of which represents X,Y coordinates of a single vertex.  This
  * array will be modified with the rotation.
  */
 void rotateTriangle(GLfloat vertexData[6]){
    // todo: actually perform rotation via translation mat + rotation mat
    // + translation mat and set vertexData contents with result.  This
    // approach requires either GLM or a custom matrix mult function since
    // those built in to OpenGL seem to assume you are not using shaders.

    // fake it for now with rotation cycle hardcoded to 90 degree rotation each frame
    switch(rotationCycle){
        // rotate 90 degrees clockwise from last pos
        case 0:
            vertexData[0] = 0.5f;
            vertexData[1] = 0.0f;
            vertexData[2] = 0.0f;
            vertexData[3] = 1.0f;
            vertexData[4] = 0.0f;
            vertexData[5] = -1.0f;
            break;
        case 1:
            vertexData[0] = 0.0f;
            vertexData[1] = -0.5f;
            vertexData[2] = 0.5f;
            vertexData[3] = 0.5f;
            vertexData[4] = -0.5f;
            vertexData[5] = 0.5f;
            break;
        case 2:
            vertexData[0] = -0.5f;
            vertexData[1] = 0.0f;
            vertexData[2] = 0.0f;
            vertexData[3] = -1.0f;
            vertexData[4] = 0.0f;
            vertexData[5] = 1.0f;
            break;
        case 3:
            vertexData[0] = 0.0f;
            vertexData[1] = 0.5f;
            vertexData[2] = -0.5f;
            vertexData[3] = -0.5f;
            vertexData[4] = 0.5f;
            vertexData[5] = -0.5f;
            break;
    }

    // advance or reset the rotation cycle
    if(rotationCycle < 4) {
        rotationCycle++;
    }else{
        rotationCycle = 0;
    }
 }
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

    // triangle vertices specifying X,Y and leaving 0 for Z,W
    // this establishes a mat4 to describe our triangle vertices and may allow for
    // simpler matrix mult with rotation matrix?
    /*
    GLfloat triangleVertices[] = { 0.0f, 0.5f, 0.0f, 0.0f,
                                   -0.5f, -0.5f, 0.0f, 0.0f,
                                   0.5f, -0.5f, 0.0f, 0.0f };
    */

    // rotation matrix
    float angle = 15;
    GLfloat matrix[16] = {
            cos(angle),-sin(angle),0,0,
            sin(angle),cos(angle),0,0,
            0,0,1,0,
            0,0,0,1
    };


    // todo: either roll my own matrix rotation or try to compile GLM...
    // not sure how else to continuously update the vertices on each frame
    //GLfloat rotatedTriangleVertices[6] = rotateTriangle();

    // tell OpenGL how to use our vertex data for rendering
    // in this case, the main point of interest is the size parameter, which
    // tells OpenGL that our data has two components (x,y coordinates) per attribute
    // todo: read in gTriangleVertices from last gl_Position vec4 so that we continuously rotate
    // OR recompute the trianglevertices at this point based on rotation and then feed them in as normal
    glVertexAttribPointer(gvPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, gTriangleVertices);

    // todo: another possibility would be to leave the tri verts as they were in the original example,
    // but then mult each vertex individually as a vec4 column by the rotation mat4 and store the X,Y parts of
    // the result in a float[6] data array like the original example used.

    checkGlError("glVertexAttribPointer");
    // specify that we want our triangle data array to be actively used for rendering calls
    glEnableVertexAttribArray(gvPositionHandle);
    checkGlError("glEnableVertexAttribArray");



    //GLuint location = glGetUniformLocation( gProgram, "rotationMatrix" );
    //glUniformMatrix2fv( location, 1, GL_FALSE, matrix );

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
