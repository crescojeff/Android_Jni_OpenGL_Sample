package com.jeffcreswell.jniopengl.ui.jnistrings;

import com.jeffcreswell.jniopengl.jni.JniHooks;
import java.util.ArrayList;
import java.util.List;

/**
 * Helper class for providing sample randomly selected strings from JNI
 *
 */
public class JniStringContent {

    /**
     * An array of sample string from JNI.
     */
    public static final List<String> ITEMS = new ArrayList<String>();

    private static final int COUNT = 1;

    static {
        // Add some sample string items to the listview content.
        for (int i = 1; i <= COUNT; i++) {
            addItem(createJniStringItem());
        }
    }

    /**
     * Adds a JNI string to our {@link #ITEMS} list for easy access by {@link com.jeffcreswell.jniopengl.ui.JniStringItemRecyclerViewAdapter}
     * @param item a String returned from JNI via {@link #createJniStringItem()}
     */
    private static void addItem(String item) {
        ITEMS.add(item);
    }

    /**
     * Reaches out to JNI lib for random generated String
     * @return randomly generated string provided by native layer
     */
    private static String createJniStringItem() {
        return JniHooks.randomString();
    }
}
