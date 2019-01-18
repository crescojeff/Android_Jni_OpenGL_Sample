package com.jeffcreswell.jniopengl;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import com.jeffcreswell.jniopengl.jni.JniHooks;
import com.jeffcreswell.jniopengl.ui.GlView;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "JniGlDemoActivity";

    private GlView mGlView;

    @Override protected void onCreate(Bundle savedData) {
        super.onCreate(savedData);
        mGlView = new GlView(getApplication());
        setContentView(mGlView);

        Log.d(TAG,"random string: "+ JniHooks.testString());
    }

    @Override protected void onPause() {
        super.onPause();
        // GLSurfaceView must be told when it will no longer be visible so it
        // can release native resources
        mGlView.onPause();
    }

    @Override protected void onResume() {
        super.onResume();
        // GLSurfaceView must be told when it will be visible (again) so it can (re)acquire resources
        // and start/continue rendering
        mGlView.onResume();
    }
}
