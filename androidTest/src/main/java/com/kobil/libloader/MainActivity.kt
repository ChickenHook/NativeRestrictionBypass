package com.kobil.libloader

import android.os.Bundle
import android.security.keystore.KeyGenParameterSpec
import android.security.keystore.KeyProperties
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import java.security.KeyStore
import java.util.*
import javax.crypto.KeyGenerator
import kotlin.experimental.and


class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        Log.d(
            "MainActivity",
            "Load symbol <" + NativeInterface.getAddr("libart.so","artFindNativeMethodRunnable") + ">"
        );
    }
}