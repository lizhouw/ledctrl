package com.android.server;

import android.content.Context;
import android.util.Slog;
import com.android.internal.flukenetworks.ILedctrlService;

public class LedctrlService extends ILedctrlService.Stub {
    private static final String TAG = "LedCtrl Service";

    LedctrlService(){
        init_native();
    }

    public void setStatus(int val){
        setstatus_native(val);
    }

    public int getStatus(){
        return getstatus_native();
    }

    private static native void init_native();
    private static native void setstatus_native(int val);
    private static native int  getstatus_native();
};

