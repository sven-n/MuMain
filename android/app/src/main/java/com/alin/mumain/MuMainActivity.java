package com.alin.mumain;

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

import org.libsdl.app.SDLActivity;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.RandomAccessFile;

/** SDL activity for the MuMain Android client. */
public final class MuMainActivity extends SDLActivity {
    private static final long UPDATE_INTERVAL_MS = 500;
    private static final int MAX_LOG_BYTES = 65_536;
    private static final int MIN_RENDER_WIDTH = 640;
    private static final int MIN_RENDER_HEIGHT = 360;
    private static final float NATIVE_RENDER_SCALE = 1.0f;

    private final Handler handler = new Handler(Looper.getMainLooper());
    private LinearLayout overlay;
    private TextView status;
    private long startMs;
    private long logBaseOffset;

    @Override
    protected String[] getLibraries() {
        return new String[] {"main"};
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        applyRenderScale();
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        startMs = System.currentTimeMillis();
        File log = new File(getFilesDir(), "MuError.log");
        logBaseOffset = log.exists() ? log.length() : 0;
        createLoadingOverlay();
        handler.postDelayed(this::pollBootLog, UPDATE_INTERVAL_MS);
    }

    private void applyRenderScale() {
        float scale = readRenderScale();
        if (scale <= 0.05f || scale >= 0.999f || mSurface == null) {
            return;
        }

        android.util.DisplayMetrics metrics = new android.util.DisplayMetrics();
        getWindowManager().getDefaultDisplay().getRealMetrics(metrics);
        int width = Math.max(MIN_RENDER_WIDTH, (int) (metrics.widthPixels * scale));
        int height = Math.max(MIN_RENDER_HEIGHT, (int) (metrics.heightPixels * scale));
        mSurface.getHolder().setFixedSize(width, height);
        android.util.Log.i("MuMainGL", "render scale " + scale + " -> " + width + "x" + height
                + " (native " + metrics.widthPixels + "x" + metrics.heightPixels + ")");
    }

    private float readRenderScale() {
        File scaleFile = new File(getFilesDir(), "render_scale.txt");
        if (!scaleFile.exists()) {
            return NATIVE_RENDER_SCALE;
        }

        try (BufferedReader reader = new BufferedReader(new FileReader(scaleFile))) {
            String value = reader.readLine();
            return value == null || value.trim().isEmpty()
                    ? NATIVE_RENDER_SCALE
                    : Float.parseFloat(value.trim());
        } catch (Exception ignored) {
            return NATIVE_RENDER_SCALE;
        }
    }

    private void createLoadingOverlay() {
        overlay = new LinearLayout(this);
        overlay.setOrientation(LinearLayout.VERTICAL);
        overlay.setGravity(Gravity.CENTER);
        overlay.setBackgroundColor(Color.argb(230, 8, 10, 20));
        overlay.setOnClickListener(view -> removeOverlay());

        TextView title = new TextView(this);
        title.setText("MU Online");
        title.setTextColor(Color.rgb(224, 186, 92));
        title.setTextSize(30);
        title.setGravity(Gravity.CENTER);
        overlay.addView(title);

        ProgressBar spinner = new ProgressBar(this);
        LinearLayout.LayoutParams spinnerParams = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
        spinnerParams.topMargin = 48;
        spinnerParams.gravity = Gravity.CENTER_HORIZONTAL;
        overlay.addView(spinner, spinnerParams);

        status = new TextView(this);
        status.setText("Starting engine…");
        status.setTextColor(Color.WHITE);
        status.setTextSize(14);
        status.setGravity(Gravity.CENTER);
        LinearLayout.LayoutParams statusParams = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
        statusParams.topMargin = 36;
        overlay.addView(status, statusParams);

        addContentView(overlay, new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));
    }

    private void pollBootLog() {
        if (overlay == null) {
            return;
        }

        String line = lastLogLineThisRun();
        long seconds = (System.currentTimeMillis() - startMs) / 1000;
        if (line != null && line.contains("Login Scene init success")) {
            removeOverlay();
            return;
        }

        String shown = line == null || line.isEmpty() ? "Loading game data…" : line;
        if (shown.length() > 64) {
            shown = shown.substring(0, 64);
        }
        status.setText("Loading — " + seconds + "s\n" + shown);
        handler.postDelayed(this::pollBootLog, UPDATE_INTERVAL_MS);
    }

    private void removeOverlay() {
        if (overlay != null && overlay.getParent() instanceof ViewGroup) {
            ((ViewGroup) overlay.getParent()).removeView(overlay);
        }
        overlay = null;
    }

    private String lastLogLineThisRun() {
        File log = new File(getFilesDir(), "MuError.log");
        if (!log.exists()) {
            return null;
        }

        try (RandomAccessFile reader = new RandomAccessFile(log, "r")) {
            long length = reader.length();
            if (length <= logBaseOffset) {
                return null;
            }

            reader.seek(logBaseOffset);
            int size = (int) Math.min(length - logBaseOffset, MAX_LOG_BYTES);
            byte[] buffer = new byte[size];
            reader.readFully(buffer);
            String[] lines = new String(buffer, "UTF-8").split("\r?\n");
            for (int index = lines.length - 1; index >= 0; index--) {
                String candidate = lines[index].trim();
                if (!candidate.isEmpty()) {
                    return candidate;
                }
            }
        } catch (Exception ignored) {
            // The next polling interval retries while the engine writes the log.
        }
        return null;
    }
}
