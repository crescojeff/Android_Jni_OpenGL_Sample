package com.jeffcreswell.jniopengl;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.jeffcreswell.jniopengl.jni.JniHooks;
import com.jeffcreswell.jniopengl.ui.GlView;
import com.jeffcreswell.jniopengl.ui.JniStringItemFragment;

public class MainActivity extends AppCompatActivity implements JniStringItemFragment.OnListFragmentInteractionListener {
    private static final String TAG = "JniGlDemoActivity";

    private GlView mGlView;
    private Button mAddStringsButton;
    private JniStringItemFragment mFragment;

    @Override protected void onCreate(Bundle savedData) {
        super.onCreate(savedData);
        setContentView(R.layout.activity_main);
        mGlView = (GlView) findViewById(R.id.my_gl_surfaceview);
        mFragment = (JniStringItemFragment) getSupportFragmentManager().findFragmentById(R.id.my_jnistrings_fragment);
        mAddStringsButton = (Button) findViewById(R.id.add_strings_btn);
        mAddStringsButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mFragment.pushDataToAdapter(JniHooks.randomString());
            }
        });
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

    @Override
    public void onListFragmentInteraction(String item) {
        Toast.makeText(this,item,Toast.LENGTH_SHORT).show();
    }
}
