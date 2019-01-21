package com.jeffcreswell.jniopengl.ui;

import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import com.jeffcreswell.jniopengl.R;
import com.jeffcreswell.jniopengl.ui.JniStringItemFragment.OnListFragmentInteractionListener;
import java.util.List;

/**
 * {@link RecyclerView.Adapter} that can display a JNI string and makes a call to the
 * specified {@link OnListFragmentInteractionListener}.
 *
 */
public class JniStringItemRecyclerViewAdapter extends RecyclerView.Adapter<JniStringItemRecyclerViewAdapter.JniStringViewHolder> {

    private final List<String> mValues;
    private final OnListFragmentInteractionListener mListener;

    public JniStringItemRecyclerViewAdapter(List<String> items, OnListFragmentInteractionListener listener) {
        mValues = items;
        mListener = listener;
    }

    @Override
    public JniStringViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext())
                .inflate(R.layout.fragment_jnistringitem, parent, false);
        return new JniStringViewHolder(view);
    }

    @Override
    public void onBindViewHolder(final JniStringViewHolder holder, int position) {
        holder.mItem = mValues.get(position);
        holder.mPosView.setText(Integer.toString(position));
        holder.mContentView.setText(holder.mItem);

        holder.mView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (null != mListener) {
                    // Notify the active callbacks interface (the activity, if the
                    // fragment is attached to one) that an item has been selected.
                    mListener.onListFragmentInteraction(holder.mItem);
                }
            }
        });
    }

    @Override
    public int getItemCount() {
        return mValues.size();
    }

    public class JniStringViewHolder extends RecyclerView.ViewHolder {
        public final View mView;
        public final TextView mPosView;
        public final TextView mContentView;
        public String mItem;

        public JniStringViewHolder(View view) {
            super(view);
            mView = view;
            mPosView = (TextView) view.findViewById(R.id.item_number);
            mContentView = (TextView) view.findViewById(R.id.content);
        }

        @Override
        public String toString() {
            return super.toString() + " '" + mContentView.getText() + "'";
        }
    }
}
