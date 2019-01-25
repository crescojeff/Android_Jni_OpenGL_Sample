package com.jeffcreswell.jniopengl.jni;

/**
 * This class provides hooks into the JNI functions of jnidemo-lib
 */
public class JniHooks {

    static{
        System.loadLibrary("jnidemo-lib");
    }

    /**
     * Initializes native OpenGL rendering context
     * @param width the width of the GL surface
     * @param height the height fo the GL surface
     * @return true if the renderer was initialized successfully, false otherwise
     */
    public static native boolean glInit(int width, int height);

    /**
     * Frees native OpenGL rendering resources
     */
    /* because of EGL layer, we don't actually need this
    public static native void glDeinit();
    */

    /**
     * Instructs native OpenGL layer to render next frame
     */
    public static native void glStepFrame();

    /**
     * Randomly generates an {adjective-ly adjective animal} string and returns it
     * @return random string
     */
    public static native String randomString();

}
