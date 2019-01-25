#include <jni.h>
#include <android/log.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale
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
 * Vertex Buffer Object we'll use to upload data to the GPU
 */
GLuint gVboID;
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
GLfloat gTriangleVertices[] = { 0.0f, 0.5f, -0.5f, -0.5f,
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

// todo:
// 1. try to use a uniform u_time to effect change in vertex coordinates over time
//    With that, a simple rotation matrix applied in the main() of the vertex shader
//    should give us continuous rotation.
//    UPDATE: turns out u_time is not a GLSL built-in, and The Book of Shaders just skips over
//    how exactly they populate it.  Looking closer at GLSL docs, uniforms act like a bridge between shader program and client,
//    which is what I've been looking for --
//      1.1 if I update u_time with current clock in renderFrame()
//      client-side and then use it as a modifier I should be able to get continuous position updates
//      mildly influenced by rotation (though it won't really be continuous rotation).
//      1.2 alternatively, I could try just keeping a modifier variable in the shader itself
//      and update it on each loop of the shader's main() (taking care to reset it when
//      it would send vertices out of rendering bounds). UPDATE: seems I need to read up on
//      how/when the components of a shader are initialized and run -- my mod variable seems
//      to only receive an update and then either never receives another or is always reset to its
//      starting value before said update such that the result is a slightly skewed slightly rotated static triangle.
//      1.3 try using a uniform in the shader to read client side (in renderFrame) the gl_Position vec4 that
//      results from the rotation applied in-shader, and use that as the new value of gTriangleVertices
// x2. try the rotation cycle approach for 'fake rotation': works!  Stupid, but technically fulfills the requirement.
// 3. try using GLM to allow for out-of-box matrix math client-side
//    and modify the vertex data given to the vertex shader from client-side as
//    open.gl/drawing does.
// 4. try applying the rot matrix directly to gl_Position (gl_Pos = rotMat * gl_Pos) after gl_pos
//    has been set with init data

/**
 * Simply sets the gl_Position for each vertex to the uploaded vertex data's
 * X,Y coordinates, specifying 0 for Z since we're in 2D at the moment.
 */
const char* gVertexShaderGLES3_noTransform = R"glsl(
        #version 150 core

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
        #version 150 core

        out vec4 outColor;

        void main()
        {
            outColor = vec4(0.917647,  0.678431, 0.917647, 1.0);
        }
)glsl";

/**
 * Basic vertex shader to use our triangle data to create anchor points, which
 * will be modified by the fragment shader later in the pipeline
 */
auto gVertexShader =
        "attribute vec4 vPosition;\n"
        "uniform mat4 rotationMatrix;"
        //"uniform float u_time;"
        //"float mod = 0.3f;"
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
        "   float angle = 45.0;\n"
        "   mat4 rotationMat = mat4(\n"
        "       cos(angle),-sin(angle),0,0,\n"
        "       sin(angle),cos(angle),0,0,\n"
        "       0,0,1,0,\n"
        "       0,0,0,1\n"
        "   );"
        /* approach #1.2: using continuously incrementing modifier on a cycle, I see the triangle transformed
           only once.  Not clear why, but I'm guessing either the shader's main() loop is not
           run on every frame and/or when the shader runs on a frame, the whole program runs
           from scratch such that mod would be reset to its starting value and only receive the first
           bump forever.
            // update the vertices on every frame with continuously incrementing
            // modifier
        "   mod += 0.001f;\n"
            // don't want to increase vertex location beyond 1.0 on any axis
        "   if(mod >= 0.5f){\n"
        "       mod = 0.0f;\n"
        "   }\n"

            // rotate the local coordinate space using modified vertices
            "gl_Position = rotationMat * (vPosition + vec4(mod,mod,mod,0.0f));\n"
            */
        /* todo 4: I don't understand why this doesn't work (black bg only)
         * whereas the gl_Pos = rotMat * vPos approach does sort of work
        "   vPosition = rotationMat * vPosition;\n"
        "   gl_Position = vPosition;\n"
        */
        "   gl_Position = vPosition;\n"
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

    /*
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
    */

    /// open.gl tutorial ///
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

    // now that data has been uploaded to the GPU, we need to tell the
    // GPU what to do with it -- that is where our shaders come in
    gProgram = createProgram(gVertexShaderGLES3_noTransform,gFragmentShaderGLES3_solidPurple);

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
    float angle = 5;
    glm::mat4 rotationMatrix = glm::mat4(
            cos(angle),-sin(angle),0,0,
            sin(angle),cos(angle),0,0,
            0,0,1,0,
            0,0,0,1
            );

    // todo 3: set the tri vert data based on rotating the previous vert data
    // such that it rotates each frame by the given angle
    glm::mul


    /* approach #2
    // todo: either roll my own matrix rotation or try to compile GLM...
    // not sure how else to continuously update the vertices on each frame
    rotateTriangle(gTriangleVertices);
    */

    // todo 1.3: how can I reference the value of vPosition via gvPosition to
    // check if it's zeroes (uninitialized) and otherwise use it to set tri verts we pass in to shader
    // as basis for rotation to new verts?
    // todo 1.3: when is the shader main actually run?
    // todo 1.3: maybe it;s possible to simply update gl_Position based on current
    // value like gl_Position = rotationMatrix * gl_Position?  Question then is how to
    // set the initial value (and only init value) of gl_Position using triangle data array?

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


    /// open.gl tutorial approach with VBO ///
    // activate our shader program
    glUseProgram(gProgram);
    // fetch the index of the named input, position
    GLint posAttrib = glGetAttribLocation(gProgram, "position");
    // instruct the vertex shader to consider our vertex data stored
    // in input position as having two components per attribute (X,Y coords)
    // with a float data type, not needing normalization to device coords,
    // stride (space between vertex attribute data) of 0, and offset (space before
    // vertex attribute data starts) of 0
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
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
