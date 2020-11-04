package com.kobil.libloader;

class NativeInterface {
    static {
        System.loadLibrary("androidTest");
    }
    public static native long getAddr(String lib, String symbol);
    public static native int load(String path);
    public static native int loadFromStorage(String path);
}
