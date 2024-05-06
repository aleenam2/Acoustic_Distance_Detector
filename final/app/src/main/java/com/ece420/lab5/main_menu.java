package com.ece420.lab5;

import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ActivityInfo;
import android.Manifest;
import android.os.Bundle;
import android.support.v4.content.ContextCompat;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;


public class main_menu extends AppCompatActivity {

    // Flag to control app behavior
    public static int appFlag = 0;
    // UI Variables
    private Button senderButton;
    private Button receiverButton;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.main_menu);
        super.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);

        // Setup Button for Sender
        senderButton = (Button) findViewById(R.id.Sender);
        senderButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                appFlag = 1;
                startActivity(new Intent(main_menu.this, Sender.class));
            }
        });

        // Setup Button for Receiver
        receiverButton = (Button) findViewById(R.id.Receiver);
        receiverButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                appFlag = 2;
                startActivity(new Intent(main_menu.this, MainActivity.class));
            }
        });
    }

}
