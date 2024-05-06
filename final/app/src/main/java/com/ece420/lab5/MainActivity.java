package com.ece420.lab5;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;
import android.media.MediaRecorder;
import android.util.Log;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.ArrayList;

import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.series.DataPoint;
import com.jjoe64.graphview.series.LineGraphSeries;
import com.jjoe64.graphview.series.PointsGraphSeries;

public class MainActivity extends Activity implements ActivityCompat.OnRequestPermissionsResultCallback {



    // UI Variables
    private Button senderButton;
    private Button receiverButton;
    private Button controlButton;
    private TextView statusView;

    // Audio Variables
    private String nativeSampleRate;
    private String nativeSampleBufSize;
    private boolean supportRecording;

    private Button sendChirp;
    private boolean isPlaying = false;
    private static final int AUDIO_ECHO_REQUEST = 0;
    private static final int FRAME_SIZE = 1024;

    private AudioRecord recorder = null;
    private static final int RECORDER_SAMPLERATE = 8000;
    private static final int RECORDER_CHANNELS = AudioFormat.CHANNEL_IN_MONO;
    private static final int RECORDER_AUDIO_ENCODING = AudioFormat.ENCODING_PCM_16BIT;
    private Thread recordingThread = null;
    private boolean isRecording = false;

    public LineGraphSeries<DataPoint> accelGraphData;
    public PointsGraphSeries<DataPoint> accelGraphSteps;
    TextView distance_view;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.main_menu);

        senderButton = (Button)findViewById(R.id.Sender);
        receiverButton = (Button)findViewById(R.id.Receiver);

//        senderButton.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                switchToMainMenu();
//            }
        if (Build.VERSION.SDK_INT >= 23) {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                    != PackageManager.PERMISSION_GRANTED || ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE)
                    != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this,
                        new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE},
                        1);
            }
        }
//        });

        senderButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // Switch to sender activity
                setContentView(R.layout.sender);
                sendChirp = (Button) findViewById(R.id.sendChirpButton);

                sendChirp.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        double fStart = 1000;
                        double fEnd = 11000;
                        double T = 1;
                        int numRepetitions = 3;
                        int sampleRate = 44100;

                        createAndPlayChirp(fStart, fEnd, T, numRepetitions, sampleRate);
                    }
                });

            }
        });

        receiverButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                switchToReceiverLayout();
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

        double[] chirpSignalTotal = new double[duration * numRepetitions];
        for (int i = 0; i < numRepetitions; i++) {
            System.arraycopy(chirpSignal, 0, chirpSignalTotal, i * duration, duration);
        }

        byte[] byteSignal = new byte[chirpSignalTotal.length * 2];
        int idx = 0;
        for (double dVal : chirpSignalTotal) {
            short val = (short) (dVal * 32767);
            byteSignal[idx++] = (byte) (val & 0x00ff);
            byteSignal[idx++] = (byte) ((val & 0xff00) >>> 8);
        }

        AudioTrack audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate, AudioFormat.CHANNEL_OUT_MONO,
                AudioFormat.ENCODING_PCM_16BIT, byteSignal.length, AudioTrack.MODE_STATIC);
        audioTrack.write(byteSignal, 0, byteSignal.length);
        audioTrack.play();
    }
    private void switchToMainMenu() {
        setContentView(R.layout.main_menu);
    }

    private void switchToReceiverLayout() {
        setContentView(R.layout.activity_main);
        controlButton = (Button)findViewById(R.id.capture_control_button);
        statusView = (TextView)findViewById(R.id.statusView);

        controlButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onEchoClick(v);
            }
        });
//        setupGraphView();


        queryNativeAudioParameters();
        updateNativeAudioUI();
        if (supportRecording) {
            createSLEngine(Integer.parseInt(nativeSampleRate), FRAME_SIZE);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        if (requestCode == AUDIO_ECHO_REQUEST) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                onEchoClick(controlButton);
            } else {
                Toast.makeText(this, R.string.error_no_permission, Toast.LENGTH_SHORT).show();
            }
        }
    }

    private void onEchoClick(View view) {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO) !=
                PackageManager.PERMISSION_GRANTED) {
            statusView.setText(getString(R.string.status_record_perm));
            ActivityCompat.requestPermissions(
                    this,
                    new String[]{Manifest.permission.RECORD_AUDIO},
                    AUDIO_ECHO_REQUEST);
            return;
        }

        if (!isPlaying) {
            if (!createSLBufferQueueAudioPlayer()) {
                statusView.setText(getString(R.string.error_player));
                return;
            }
            if (!createAudioRecorder()) {
                deleteSLBufferQueueAudioPlayer();
                statusView.setText(getString(R.string.error_recorder));
                return;
            }
            startRecording();
//            startPlay();
            statusView.setText(getString(R.string.status_echoing));

            Toast.makeText(this, "Recording started", Toast.LENGTH_SHORT).show();
        } else {
//            stopPlay();
            stopRecording();
            transferAudioFile("/sdcard/voice8K16bitmono.pcm");
            updateNativeAudioUI();
            deleteAudioRecorder();
            deleteSLBufferQueueAudioPlayer();
        }
        isPlaying = !isPlaying;
        controlButton.setText(getString(isPlaying ? R.string.StopEcho : R.string.StartEcho));
    }
    private void stopRecording() {
        // stops the recording activity
        if (null != recorder) {
            isRecording = false;
            recorder.stop();
            recorder.release();
            recorder = null;
            recordingThread = null;
        }
    }
    public void getLowLatencyParameters(View view) {
        updateNativeAudioUI();
        return;
    }
    int BufferElements2Rec = 1024; // want to play 2048 (2K) since 2 bytes we use only 1024
    int BytesPerElement = 2; // 2 bytes in 16bit format


//    reference to : https://stackoverflow.com/questions/8499042/android-audiorecord-example
    private void startRecording() {

        recorder = new AudioRecord(MediaRecorder.AudioSource.MIC,
                RECORDER_SAMPLERATE, RECORDER_CHANNELS,
                RECORDER_AUDIO_ENCODING, BufferElements2Rec * BytesPerElement);

        recorder.startRecording();
        isRecording = true;
        recordingThread = new Thread(new Runnable() {
            public void run() {
                writeAudioDataToFile();
            }
        }, "AudioRecorder Thread");
        recordingThread.start();
    }

    private byte[] short2byte(short[] sData) {
        int shortArrsize = sData.length;
        byte[] bytes = new byte[shortArrsize * 2];
        for (int i = 0; i < shortArrsize; i++) {
            bytes[i * 2] = (byte) (sData[i] & 0x00FF);
            bytes[(i * 2) + 1] = (byte) (sData[i] >> 8);
            sData[i] = 0;
        }
        return bytes;}
    private void writeAudioDataToFile() {
        // Write the output audio in byte

        String filePath = "/sdcard/voice8K16bitmono.pcm";
        short sData[] = new short[BufferElements2Rec];

        FileOutputStream os = null;
        try {
            os = new FileOutputStream(filePath);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }

        while (isRecording) {
            // gets the voice output from microphone to byte format

            recorder.read(sData, 0, BufferElements2Rec);
            System.out.println("Short writing to file" + sData.toString());
            try {
                // // writes the data to file from buffer
                // // stores the voice buffer
                byte bData[] = short2byte(sData);
                os.write(bData, 0, BufferElements2Rec * BytesPerElement);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        try {
            os.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    public class audio_data {
        int sample_rate;
        ArrayList<Float> samples;
        public audio_data(int sample_rate){
            this.sample_rate = sample_rate;
            this.samples = new ArrayList<>();
        }
        public void store(byte[]buffer, int read){
            int numSamples = read / 2;
            for (int i = 0; i < numSamples; i++) {
//                if (samples.size() < 10240) {
                int low = buffer[2 * i] & 0xFF;
                int high = buffer[2 * i + 1] & 0xFF;
                float data = (float) (high << 8 | low);
//                Log.d("AudioDataStore", "Attempting to add sample: " + data);
                samples.add(data);
//                Log.d("AudioDataStore", "Sample added successfully. Total samples: " + samples.size());
//                }
            }
        }
        public ArrayList<Float> getSamples() {
            return samples;
        }
        public void empty_list() {
            samples.clear();
        }
        public native int processSamplesNative(ArrayList<Float> samples);
    }
    private void transferAudioFile(String filePath) {
        int sampleRate = 8000;
//        int audioFormat = AudioFormat.ENCODING_PCM_16BIT;
//        int channelConfig = AudioFormat.CHANNEL_OUT_MONO;
//
//        int minBufferSize = AudioTrack.getMinBufferSize(sampleRate, channelConfig, audioFormat);

        FileInputStream is = null;
        try {
            is = new FileInputStream(filePath);
            byte[] buffer = new byte[1024];
            int read;
            audio_data data = new audio_data(sampleRate);
            while ((read = is.read(buffer)) != -1) {
                data.store(buffer,read);
            }
            ArrayList<Float> data_cur = data.getSamples();
            float[] fft = getDataFromCpp(data_cur);
            float distance = data.processSamplesNative(data_cur);

            GraphView graph = (GraphView) findViewById(R.id.graph);

            // Set up touch listeners as previously describe
            if (graph == null) {
                Log.e("MainActivity", "GraphView is null!");
                return;
            }
            if (accelGraphData == null) {
                accelGraphData = new LineGraphSeries<>();
            }
            accelGraphData = new LineGraphSeries<>();
            int len = fft.length;
            double max = len - 1;
            for (int i = 0; i < len; i++) {
                double x = 4*(max / (len - 1)) * i;  // Calculate the x value
                double y = fft[i];  // y-value from FFT data
                accelGraphData.appendData(new DataPoint(x, y), true, len);
            }

            graph.removeAllSeries();
            graph.addSeries(accelGraphData);
            graph.getViewport().setXAxisBoundsManual(true);
            graph.getViewport().setMinX(0);
            graph.getViewport().setMaxX(max);
            setupGraphViewListeners(graph);
            TextView distance_view = (TextView) findViewById(R.id.textView2);
            if (distance_view != null) {
                distance_view.setText("Distance = " + ((distance/100)) +"cm");
            }
            data.empty_list();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (is != null) {
                try {
                    is.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private void queryNativeAudioParameters() {
        AudioManager myAudioMgr = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        nativeSampleRate = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
        nativeSampleBufSize = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
        int recBufSize = AudioRecord.getMinBufferSize(
                Integer.parseInt(nativeSampleRate),
                AudioFormat.CHANNEL_IN_MONO,
                AudioFormat.ENCODING_PCM_16BIT);
        supportRecording = recBufSize != AudioRecord.ERROR && recBufSize != AudioRecord.ERROR_BAD_VALUE;
    }

    private void updateNativeAudioUI() {
        if (!supportRecording) {
            statusView.setText(getString(R.string.error_no_mic));
            controlButton.setEnabled(false);
            return;
        }
        statusView.setText("nativeSampleRate    = " + nativeSampleRate + "\n" +
                "nativeSampleBufSize = " + nativeSampleBufSize + "\n");
    }
    private double xStart = -1;
    private double xEnd = -1;

    private void setupGraphViewListeners(GraphView graph) {
        graph.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch (event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        xStart = screenXToGraphX(graph, event.getX());
                        return true;
                    case MotionEvent.ACTION_UP:
                        xEnd = screenXToGraphX(graph, event.getX());
                        if (xStart != -1 && xEnd != -1) {
                            double length = Math.abs(xEnd - xStart);
                            showLength(length);
                        }
                        return true;
                }
                return false;
            }
        });
    }

    private double screenXToGraphX(GraphView graph, float screenX) {
        // Get the visible bounds
        double minX = graph.getViewport().getMinX(false);
        double maxX = graph.getViewport().getMaxX(false);

        // Get the width of the graph view
        int width = graph.getGraphContentWidth();

        // Calculate the proportion of the screenX relative to the graph width
        double proportion = (screenX - graph.getGraphContentLeft()) / (double) width;

        // Calculate the corresponding graph X coordinate
        return minX + proportion * (maxX - minX);
    }
    private void showLength(double length) {
        TextView distance_view = (TextView) findViewById(R.id.textView);
        distance_view.setText("Distance = " + 4*length/(2*4800) + "m");
    }


    static {
        System.loadLibrary("echo");
    }

    public static native void createSLEngine(int rate, int framesPerBuf);
    public static native void deleteSLEngine();
    public static native boolean createSLBufferQueueAudioPlayer();
    public static native void deleteSLBufferQueueAudioPlayer();
    public static native boolean createAudioRecorder();
    public static native void deleteAudioRecorder();
    public static native void startPlay();
    public static native void stopPlay();

    private native float[] getDataFromCpp(ArrayList<Float> samples);
}
