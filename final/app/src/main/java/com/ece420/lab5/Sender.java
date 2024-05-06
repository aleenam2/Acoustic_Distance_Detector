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
import com.ece420.lab5.R;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

public class Sender extends AppCompatActivity{

    private Button sendChirp;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // Set the content to a layout that contains your button. You need to create this layout file.
        setContentView(R.layout.sender);

        sendChirp = (Button) findViewById(R.id.sendChirpButton);
        // Set up any onClickListener for sendChirp button if needed.

        sendChirp.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // Example values for chirp parameters
                double fStart = 1000; // Start frequency in Hz
                double fEnd = 11000; // End frequency in Hz
                double T = 1; // Duration of one chirp in seconds
                int numRepetitions = 3; // Number of times to repeat the chirp
                int sampleRate = 44100; // Sampling rate in Hz

                createAndPlayChirp(fStart, fEnd, T, numRepetitions, sampleRate);
            }
        });
    }



    public void createAndPlayChirp(double fStart, double fEnd, double T, int numRepetitions, int sampleRate) {
        int duration = (int) (T * sampleRate);
        double B = fEnd - fStart;
        double[] t = new double[duration];
        for (int i = 0; i < duration; i++) {
            t[i] = i / (double) sampleRate;
        }

        double[] chirpSignal = new double[duration];
        for (int i = 0; i < duration; i++) {
            chirpSignal[i] = Math.cos(2 * Math.PI * (fStart * t[i] + 0.5 * B * t[i] * t[i] / T));
        }

        // Repeat the chirp signal as specified by numRepetitions
        double[] chirpSignalTotal = new double[duration * numRepetitions];
        for (int i = 0; i < numRepetitions; i++) {
            System.arraycopy(chirpSignal, 0, chirpSignalTotal, i * duration, duration);
        }

        // Convert double array to byte array
        byte[] byteSignal = new byte[chirpSignalTotal.length * 2];
        int idx = 0;
        for (double dVal : chirpSignalTotal) {
            short val = (short) (dVal * 32767);
            byteSignal[idx++] = (byte) (val & 0x00ff);
            byteSignal[idx++] = (byte) ((val & 0xff00) >>> 8);
        }

        // Play the chirp using AudioTrack
        AudioTrack audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate, AudioFormat.CHANNEL_OUT_MONO,
                AudioFormat.ENCODING_PCM_16BIT, byteSignal.length, AudioTrack.MODE_STATIC);
        audioTrack.write(byteSignal, 0, byteSignal.length);
        audioTrack.play();
    }

}
