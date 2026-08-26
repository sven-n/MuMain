package com.alin.mumain;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.Gravity;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;

/** Shows loading feedback while libmain's global constructors read game data. */
public final class LoadingActivity extends Activity {
    private static final long UPDATE_INTERVAL_MS = 500;
    private static final int COPY_BUFFER_SIZE = 8192;

    private final Handler handler = new Handler(Looper.getMainLooper());
    private TextView status;
    private long startMs;
    private volatile boolean libraryLoaded;
    private volatile String libraryError;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        startMs = System.currentTimeMillis();
        setContentView(createLoadingView());

        Thread loader = new Thread(this::loadEngine, "libmain-loader");
        loader.start();
        handler.postDelayed(this::updateStatus, UPDATE_INTERVAL_MS);
    }

    private LinearLayout createLoadingView() {
        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setGravity(Gravity.CENTER);
        root.setBackgroundColor(Color.rgb(8, 10, 20));

        TextView title = new TextView(this);
        title.setText("MU Online");
        title.setTextColor(Color.rgb(224, 186, 92));
        title.setTextSize(32);
        title.setGravity(Gravity.CENTER);
        root.addView(title);

        ProgressBar spinner = new ProgressBar(this);
        LinearLayout.LayoutParams spinnerParams = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
        spinnerParams.topMargin = 48;
        spinnerParams.gravity = Gravity.CENTER_HORIZONTAL;
        root.addView(spinner, spinnerParams);

        status = new TextView(this);
        status.setText("Loading engine and game data…");
        status.setTextColor(Color.WHITE);
        status.setTextSize(14);
        status.setGravity(Gravity.CENTER);
        LinearLayout.LayoutParams statusParams = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
        statusParams.topMargin = 36;
        root.addView(status, statusParams);
        return root;
    }

    private void loadEngine() {
        try {
            prepareData();
            System.loadLibrary("main");
            libraryLoaded = true;
        } catch (Throwable error) {
            libraryError = error.getMessage();
        }
    }

    private void updateStatus() {
        long seconds = (System.currentTimeMillis() - startMs) / 1000;
        if (libraryError != null) {
            status.setText("Engine failed to load:\n" + libraryError);
            return;
        }
        if (libraryLoaded) {
            startActivity(new Intent(this, MuMainActivity.class));
            finish();
            return;
        }

        status.setText("Loading engine and game data… " + seconds + "s\n"
                + "(first boot takes a minute or two)");
        handler.postDelayed(this::updateStatus, UPDATE_INTERVAL_MS);
    }

    private void prepareData() {
        try {
            File filesDirectory = getFilesDir();
            linkSharedDataWhenNeeded(filesDirectory);
            copySharedConfigurationWhenNewer(filesDirectory);
        } catch (Exception ignored) {
            // MuMain reports missing data through MuError.log and its loading UI.
        }
    }

    private void linkSharedDataWhenNeeded(File filesDirectory) throws Exception {
        File internalData = new File(filesDirectory, "Data");
        File sharedData = new File("/sdcard/mumain/Data");
        if (!internalData.exists() && sharedData.exists()) {
            android.system.Os.symlink(sharedData.getAbsolutePath(), internalData.getAbsolutePath());
        }
    }

    private void copySharedConfigurationWhenNewer(File filesDirectory) throws Exception {
        File sharedConfiguration = new File("/sdcard/mumain/config.ini");
        File internalConfiguration = new File(filesDirectory, "config.ini");
        if (!sharedConfiguration.exists()
                || (internalConfiguration.exists()
                && sharedConfiguration.lastModified() <= internalConfiguration.lastModified())) {
            return;
        }

        try (FileInputStream input = new FileInputStream(sharedConfiguration);
             FileOutputStream output = new FileOutputStream(internalConfiguration)) {
            byte[] buffer = new byte[COPY_BUFFER_SIZE];
            int bytesRead;
            while ((bytesRead = input.read(buffer)) > 0) {
                output.write(buffer, 0, bytesRead);
            }
        }
    }
}
