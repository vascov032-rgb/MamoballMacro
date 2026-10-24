package com.flork.macro;

import android.app.Activity;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.graphics.drawable.GradientDrawable;
import android.os.Handler;
import android.os.Looper;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.FrameLayout;

public class FlorkOverlay {
    private static WindowManager wm;
    private static FrameLayout root;
    private static Button btn;
    private static WindowManager.LayoutParams params;
    private static boolean visible = false;
    private static Handler handler = new Handler(Looper.getMainLooper());
    private static boolean isHolding = false;
    
    static {
        // Carrega a .so direto da pasta do módulo (evita erro do sistema)
        try {
            System.load("/data/adb/modules/mamoball_macro/zygisk/libflork_android.so");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    public static native void nativeInstall();
    public static native void nativeStartMacro();
    public static native void nativeUpdate();

    public static void install(Activity activity) {
        activity.runOnUiThread(() -> {
            if (root != null) return;
            
            wm = (WindowManager) activity.getSystemService(Activity.WINDOW_SERVICE);
            root = new FrameLayout(activity);
            btn = new Button(activity);
            btn.setText("KICK");
            btn.setTextColor(Color.WHITE);
            btn.setTextSize(18);
            
            GradientDrawable bg = new GradientDrawable();
            bg.setShape(GradientDrawable.OVAL);
            bg.setColor(Color.parseColor("#8000FF00"));
            bg.setStroke(3, Color.GREEN);
            btn.setBackground(bg);
            btn.setAlpha(0f);

            FrameLayout.LayoutParams btnParams = new FrameLayout.LayoutParams(200, 200);
            btnParams.gravity = Gravity.BOTTOM | Gravity.RIGHT;
            btnParams.setMargins(0, 0, 100, 300);
            root.addView(btn, btnParams);

            root.setOnTouchListener((v, event) -> {
                if (event.getActionMasked() == MotionEvent.ACTION_POINTER_DOWN && event.getPointerCount() == 4) {
                    visible = !visible;
                    btn.setAlpha(visible ? 1f : 0f);
                }
                return false;
            });

            btn.setOnTouchListener(new View.OnTouchListener() {
                @Override
                public boolean onTouch(View v, MotionEvent event) {
                    switch (event.getAction()) {
                        case MotionEvent.ACTION_DOWN:
                            if (btn.getAlpha() == 0f) {
                                isHolding = true;
                                handler.postDelayed(holdRunnable, 300);
                            }
                            return true;
                        case MotionEvent.ACTION_UP:
                            handler.removeCallbacks(holdRunnable);
                            if (isHolding) {
                                isHolding = false;
                            } else {
                                triggerMacro();
                            }
                            return true;
                    }
                    return false;
                }
            });

            params = new WindowManager.LayoutParams(
                    WindowManager.LayoutParams.MATCH_PARENT, WindowManager.LayoutParams.MATCH_PARENT,
                    WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY,
                    WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE |
                    WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL |
                    WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN,
                    PixelFormat.TRANSLUCENT);
            
            wm.addView(root, params);
            nativeInstall();
        });
    }

    private static Runnable holdRunnable = new Runnable() {
        @Override
        public void run() {
            if (isHolding) {
                triggerMacro();
                handler.postDelayed(this, 10);
            }
        }
    };

    private static void triggerMacro() {
        try {
            nativeStartMacro();
        } catch (Exception e) {}
    }
}
