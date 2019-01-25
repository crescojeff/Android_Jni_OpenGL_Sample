package com.jeffcreswell.jniopengl.ui;

import android.content.Context;
import android.content.res.Resources;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;

import com.jeffcreswell.jniopengl.jni.JniHooks;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

public class GlView extends GLSurfaceView {
    private static String TAG = "GlView-JniDemo";
    public GlView(Context context, AttributeSet attrs) {
        super(context,attrs);
        // set up EGL for GLESv3
        setEGLContextClientVersion(3);
        // rendering context with EGL metadata, such as EGL client API version
        setEGLContextFactory(new ContextFactory());

        // select EGL configuration.  This ensures
        // that necessary hardware resources are allocated to render according to our specifications,
        // and that we fail fast if the hardware can't support same.
        // EGL is basically the glue that binds OpenGL rendering to the underlying platform's window system.
        // For demo purposes, we'll go with 8 bits/channel surface configuration.
        setEGLConfigChooser(new ConfigChooser(8,8,8,8,0,0));

        // the renderer is responsible for drawing each frame
        setRenderer(new Renderer());
    }

    private static class ContextFactory implements GLSurfaceView.EGLContextFactory {
        private static int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
        public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig eglConfig) {
            Log.w(TAG, "creating OpenGL ES 3.0 context");
            checkEglError("Before eglCreateContext, our EGL interface says:", egl);
            int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL10.EGL_NONE };
            EGLContext context = egl.eglCreateContext(display, eglConfig, EGL10.EGL_NO_CONTEXT, attrib_list);
            checkEglError("After eglCreateContext, our EGL interface says:", egl);
            return context;
        }

        public void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context) {
            egl.eglDestroyContext(display, context);
        }
    }

    private static void checkEglError(String prompt, EGL10 egl) {
        int error;
        while ((error = egl.eglGetError()) != EGL10.EGL_SUCCESS) {
            Log.e(TAG, String.format("%s: EGL error: 0x%x", prompt, error));
        }
    }

    private static class ConfigChooser implements GLSurfaceView.EGLConfigChooser {

        private int mRedSize;
        private int mGreenSize;
        private int mBlueSize;
        private int mAlphaSize;
        private int mDepthSize;
        private int mStencilSize;
        private int[] mValue = new int[1];

        public ConfigChooser(int r, int g, int b, int a, int depth, int stencil) {
            mRedSize = r;
            mGreenSize = g;
            mBlueSize = b;
            mAlphaSize = a;
            mDepthSize = depth;
            mStencilSize = stencil;
        }

        /* This EGL config specification is used to specify 2.0 rendering.
         * We use a minimum size of 4 bits for red/green/blue, but will
         * perform actual matching in chooseConfig() below.
         */
        private static int EGL_OPENGL_ES2_BIT = 4;
        private static int[] s_configAttribs2 =
                {
                        EGL10.EGL_RED_SIZE, 4,
                        EGL10.EGL_GREEN_SIZE, 4,
                        EGL10.EGL_BLUE_SIZE, 4,
                        EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                        EGL10.EGL_NONE
                };

        /**
         * Generates a set of potential EGL config matches, then sends them to {@link #chooseConfig(EGL10, EGLDisplay, EGLConfig[])}
         * so that the best fit can be returned
         * @param egl EGL API interface
         * @param display native display object
         * @return a best-fit EGL config
         */
        public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {

            /* Get the number of minimally matching EGL configurations
             */
            int[] num_config = new int[1];
            egl.eglChooseConfig(display, s_configAttribs2, null, 0, num_config);

            int numConfigs = num_config[0];

            if (numConfigs <= 0) {
                throw new IllegalArgumentException("No configs match configSpec");
            }

            /* Allocate then read the array of minimally matching EGL configs
             */
            EGLConfig[] configs = new EGLConfig[numConfigs];
            egl.eglChooseConfig(display, s_configAttribs2, configs, numConfigs, num_config);

            /* Now return the "best" one
             */
            return chooseConfig(egl, display, configs);
        }

        /**
         * This method steps through all the available EGL configurations given in configs and attempts
         * to match the attributes therein to our desired attributes, or at least come up with a configuration
         * that can handle our desired attributes.
         * @param egl EGL API interface
         * @param display wrapper for native EGL display object
         * @param configs available EGL configurations
         * @return a best-fit EGL config
         */
        public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display,
                                      EGLConfig[] configs) {
            for(EGLConfig config : configs) {
                int depthAttribute = findConfigAttrib(egl, display, config,
                        EGL10.EGL_DEPTH_SIZE, 0);
                int stencilAttribute = findConfigAttrib(egl, display, config,
                        EGL10.EGL_STENCIL_SIZE, 0);

                // We need at least mDepthSize and mStencilSize bits
                if (depthAttribute < mDepthSize || stencilAttribute < mStencilSize)
                    continue;

                // We want an exact match on bitness for red/green/blue/alpha color channels
                int redChannel = findConfigAttrib(egl, display, config,
                        EGL10.EGL_RED_SIZE, 0);
                int greenChannel = findConfigAttrib(egl, display, config,
                        EGL10.EGL_GREEN_SIZE, 0);
                int blueChannel = findConfigAttrib(egl, display, config,
                        EGL10.EGL_BLUE_SIZE, 0);
                int alphaChannel = findConfigAttrib(egl, display, config,
                        EGL10.EGL_ALPHA_SIZE, 0);

                if (redChannel == mRedSize && greenChannel == mGreenSize && blueChannel == mBlueSize && alphaChannel == mAlphaSize)
                    return config;
            }
            return null;
        }

        private int findConfigAttrib(EGL10 egl, EGLDisplay display,
                                     EGLConfig config, int attribute, int defaultValue) {

            if (egl.eglGetConfigAttrib(display, config, attribute, mValue)) {
                return mValue[0];
            }
            return defaultValue;
        }
    }

    private static class Renderer implements GLSurfaceView.Renderer {
        public void onDrawFrame(GL10 gl) {
            JniHooks.glStepFrame();
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            Log.d(TAG,"onSurfaceChanged; sending width: "+width+" and height: "+height
                    +". For reference, display width is "+ Resources.getSystem().getDisplayMetrics().widthPixels
                    +" and display height is "+Resources.getSystem().getDisplayMetrics().heightPixels);
            JniHooks.glInit(width,height);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            Log.d(TAG,"surface created with config {"+config.toString()+"}");
        }
    }
}
