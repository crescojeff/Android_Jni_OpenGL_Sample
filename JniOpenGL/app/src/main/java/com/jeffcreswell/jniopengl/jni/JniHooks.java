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
     * @return true if the renderer was initialized successfully, false otherwise
     */
    public static native boolean glInit();

    /**
     * Instructs native OpenGL layer to render next frame
     */
    public static native void glStepFrame();

}
