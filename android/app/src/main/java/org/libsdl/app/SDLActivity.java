package org.libsdl.app;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;

public class SDLActivity extends Activity implements SurfaceHolder.Callback {

    protected static SDLActivity mSingleton;
    protected SurfaceView mSurface;

    public static native int nativeInit(Object arguments);
    public static native void nativeQuit();
    public static native void onNativeSurfaceCreated();
    public static native void onNativeSurfaceChanged();
    public static native void onNativeSurfaceDestroyed();
    public static native void onNativeKeyDown(int keycode);
    public static native void onNativeKeyUp(int keycode);
    public static native void onNativeTouch(int touchDevId, int pointerFingerId, int action, float x, float y, float p);

    protected String getMainSharedObject() {
        return "libsyphon_filter.so";
    }

    protected String[] getMainLibraries() {
        return new String[] { "syphon_filter" };
    }

    protected String[] getArguments() {
        return new String[0];
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mSingleton = this;

        for (String lib : getMainLibraries()) {
            try {
                System.loadLibrary(lib);
            } catch (UnsatisfiedLinkError e) {
                e.printStackTrace();
            }
        }

        mSurface = new SurfaceView(this);
        mSurface.getHolder().addCallback(this);
        setContentView(mSurface);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        try {
            onNativeSurfaceCreated();
        } catch (UnsatisfiedLinkError e) {}
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        try {
            onNativeSurfaceChanged();
        } catch (UnsatisfiedLinkError e) {}
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        try {
            onNativeSurfaceDestroyed();
        } catch (UnsatisfiedLinkError e) {}
    }

    public static Context getContext() {
        return mSingleton;
    }
}
