//
// Created by daran on 1/12/2017 to be used in ECE420 Sp17 for the first time.
// Modified by dwang49 on 1/1/2018 to adapt to Android 7.0 and Shield Tablet updates.
//

#include <jni.h>
#include "ece420_main.h"
#include "ece420_lib.h"
#include "kiss_fft/kiss_fft.h"
#include <vector>
#include <numeric>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <cmath>

// Student Variables
#define EPOCH_PEAK_REGION_WIGGLE 30
#define VOICED_THRESHOLD 200000000
#define FRAME_SIZE 1024
#define BUFFER_SIZE (3 * FRAME_SIZE)
#define F_S 48000
float bufferIn[BUFFER_SIZE] = {};
float bufferOut[BUFFER_SIZE] = {};
int newEpochIdx = FRAME_SIZE;

// We have two variables here to ensure that we never change the desired frequency while
// processing a frame. Thread synchronization, etc. Setting to 300 is only an initializer.
int FREQ_NEW_ANDROID = 300;
int FREQ_NEW = 300;
//extern "C"
//JNIEXPORT jfloatArray JNICALL
//Java_com_ece420_lab5_MainActivity_getDataFromCpp(JNIEnv *env, jobject audio, jobject arrayList) {
//    // TODO: implement getDataFromCpp()
//
//    jclass arrayListClass = env->FindClass("java/util/ArrayList");
//    jmethodID sizeMethod = env->GetMethodID(arrayListClass, "size", "()I");
//    jmethodID getMethod = env->GetMethodID(arrayListClass, "get", "(I)Ljava/lang/Object;");
//    jclass floatClass = env->FindClass("java/lang/Float");
//    jmethodID floatValueMethod = env->GetMethodID(floatClass, "floatValue", "()F");
//    std::vector<float> audioVector;
//    if ((sizeMethod == nullptr) ||(getMethod== nullptr) ||(floatValueMethod ==nullptr)) {
//        return nullptr;
//    }
//    jint size = env->CallIntMethod(arrayList, sizeMethod);
//    jfloatArray result = env->NewFloatArray(size);
//    for (int i = 0; i < size; i++) {
//        jobject floatObject = env->CallObjectMethod(arrayList, getMethod, i);
//        if (env->ExceptionCheck()) {
//            env->ExceptionClear();
//            return nullptr;
//        }
//
//        jfloat floatValue = env->CallFloatMethod(floatObject, floatValueMethod);
//        audioVector.push_back(floatValue);
//        env->DeleteLocalRef(floatObject);
//    }
//    env->DeleteLocalRef(arrayListClass);
//    env->DeleteLocalRef(floatClass);
//    chop_signal(audioVector);
//    std::vector<float> fft = window_fft_distance(audioVector);
//    env->SetFloatArrayRegion(result, 0, fft.size(), &fft[0]);
//    return result;
//}
//extern "C" JNIEXPORT jfloatArray JNICALL
//Java_com_ece420_lab5_MainActivity_getDataFromCpp(JNIEnv *env, jobject thiz, jobject arrayList) {
//    jclass arrayListClass = env->FindClass("java/util/ArrayList");
//    jmethodID sizeMethod = env->GetMethodID(arrayListClass, "size", "()I");
//    jmethodID getMethod = env->GetMethodID(arrayListClass, "get", "(I)Ljava/lang/Object;");
//    jclass floatClass = env->FindClass("java/lang/Float");
//    jmethodID floatValueMethod = env->GetMethodID(floatClass, "floatValue", "()F");
//    if ((sizeMethod == nullptr) ||(getMethod== nullptr) ||(floatValueMethod ==nullptr)){
//        env->DeleteLocalRef(arrayListClass);
//        env->DeleteLocalRef(floatClass);
//        return nullptr;
//    }
//    jint size = env->CallIntMethod(arrayList, sizeMethod);
//    std::vector<float> audioVector;
//    audioVector.reserve(size);
//    for (int i = 0; i < size; i++) {
//        jobject floatObject = env->CallObjectMethod(arrayList, getMethod, i);
//        if (env->ExceptionCheck()) {
//            env->ExceptionClear();
//            env->DeleteLocalRef(floatObject);
//            env->DeleteLocalRef(arrayListClass);
//            env->DeleteLocalRef(floatClass);
//            return nullptr;
//        }
//
//        jfloat floatValue = env->CallFloatMethod(floatObject, floatValueMethod);
//        audioVector.push_back(floatValue);
//        env->DeleteLocalRef(floatObject);
//    }
//    env->DeleteLocalRef(arrayListClass);
//    env->DeleteLocalRef(floatClass);
//    chop_signal(audioVector);
//    std::vector<float> fft = window_fft_distance(audioVector);
//    jfloatArray result = env->NewFloatArray(fft.size());
//    if (result == nullptr) {
//        // Out of memory error thrown by JVM
//        env->DeleteLocalRef(arrayListClass);
//        env->DeleteLocalRef(floatClass);
//        return nullptr;
//    }
//
//    // Convert std::vector<float> to jfloatArray
//    env->SetFloatArrayRegion(result, 0, fft.size(), &fft[0]);
//
//    // Cleanup
//    env->DeleteLocalRef(arrayListClass);
//    env->DeleteLocalRef(floatClass);
//
//    return result;
//}
extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_ece420_lab5_MainActivity_getDataFromCpp(JNIEnv *env, jobject thiz, jobject arrayList) {
    jclass arrayListClass = env->FindClass("java/util/ArrayList");
    if (arrayListClass == nullptr) return nullptr;  // Class not found

    jmethodID sizeMethod = env->GetMethodID(arrayListClass, "size", "()I");
    jmethodID getMethod = env->GetMethodID(arrayListClass, "get", "(I)Ljava/lang/Object;");
    if (sizeMethod == nullptr || getMethod == nullptr) {
        env->DeleteLocalRef(arrayListClass);
        return nullptr;  // Method ID not found
    }

    jclass floatClass = env->FindClass("java/lang/Float");
    if (floatClass == nullptr) {
        env->DeleteLocalRef(arrayListClass);
        return nullptr;  // Class not found
    }

    jmethodID floatValueMethod = env->GetMethodID(floatClass, "floatValue", "()F");
    if (floatValueMethod == nullptr) {
        env->DeleteLocalRef(arrayListClass);
        env->DeleteLocalRef(floatClass);
        return nullptr;  // Method ID not found
    }

    jint size = env->CallIntMethod(arrayList, sizeMethod);
    std::vector<float> audioVector;
    audioVector.reserve(size);

    for (int i = 0; i < size; i++) {
        jobject floatObject = env->CallObjectMethod(arrayList, getMethod, i);
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
            env->DeleteLocalRef(arrayListClass);
            env->DeleteLocalRef(floatClass);
            return nullptr;
        }

        jfloat floatValue = env->CallFloatMethod(floatObject, floatValueMethod);
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
            env->DeleteLocalRef(arrayListClass);
            env->DeleteLocalRef(floatClass);
            return nullptr;
        }

        audioVector.push_back(floatValue);
        env->DeleteLocalRef(floatObject);
    }

    // Assuming chop_signal and window_fft_distance are valid and do not throw exceptions
    chop_signal(audioVector);
    std::vector<float> fft = window_fft_distance(audioVector);

    jfloatArray result = env->NewFloatArray(fft.size());
    if (result == nullptr) {
        env->DeleteLocalRef(arrayListClass);
        env->DeleteLocalRef(floatClass);
        return nullptr;  // Out of memory error thrown by JVM
    }

    env->SetFloatArrayRegion(result, 0, fft.size(), &fft[0]);

    env->DeleteLocalRef(arrayListClass);
    env->DeleteLocalRef(floatClass);
    return result;
}
extern "C" JNIEXPORT jint JNICALL
Java_com_ece420_lab5_MainActivity_00024audio_1data_processSamplesNative(JNIEnv *env, jobject audio, jobject arrayList) {
    jclass arrayListClass = env->FindClass("java/util/ArrayList");
    jmethodID sizeMethod = env->GetMethodID(arrayListClass, "size", "()I");
    jmethodID getMethod = env->GetMethodID(arrayListClass, "get", "(I)Ljava/lang/Object;");
    jclass floatClass = env->FindClass("java/lang/Float");
    jmethodID floatValueMethod = env->GetMethodID(floatClass, "floatValue", "()F");
    std::vector<float> audioVector;
    if ((sizeMethod == nullptr) ||(getMethod== nullptr) ||(floatValueMethod ==nullptr)){
        return -1;
    }
    jint size = env->CallIntMethod(arrayList, sizeMethod);
    for (int i = 0; i < size; i++) {
        jobject floatObject = env->CallObjectMethod(arrayList, getMethod, i);
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
            return -1;
        }

        jfloat floatValue = env->CallFloatMethod(floatObject, floatValueMethod);
        audioVector.push_back(floatValue);
        env->DeleteLocalRef(floatObject);
    }
    env->DeleteLocalRef(arrayListClass);
    env->DeleteLocalRef(floatClass);
    std::vector<int>::size_type vector_size = audioVector.size();
    std::vector<int> distance = find_distance(audioVector,vector_size);
    float dis = 3.0;
    if (distance.size() != 0) {
        dis = distance[1]-distance[0];
    }
    return dis;
}

std::vector<int> find_distance(std::vector<float>& audio_data, int N) {
    chop_signal(audio_data);
    std::vector<float> fft = window_fft_distance(audio_data);
    std::vector<int> peak = find_peak(fft);
    return peak;
}


void chop_signal(std::vector<float>& data) {
//    double fs = 48000;
    int start = -1;
    int frame = 480;
    std::vector<float> noise(data.end() - 500, data.end());
    double noise_threshold = std::sqrt(std::inner_product(noise.begin(), noise.end(), noise.begin(), 0.0));
    int frame_idx = 0;
    int start_frame = -1;
    if (start == -1) {
        while ((frame_idx + frame) < data.size()) {
            auto cur_frame = std::vector<float>(data.begin() + frame_idx, data.begin() + frame_idx + frame);
            if (std::sqrt(std::inner_product(cur_frame.begin(), cur_frame.end(), cur_frame.begin(), 0.0)) > noise_threshold) {
                start_frame = frame_idx;
                break;
            }
            frame_idx += frame;
        }
    } else {
        start_frame = start;
    }
    data.erase(data.begin(), data.begin() + start_frame);
}


std::vector<float> window_fft_distance(const std::vector<float>& data_r) {
    double fs = 48000;
    double f_end = 11000;
    double f_start = 1000;
    double T = 1.0;
    int num_repetitions = 3;
    double B = f_end - f_start;
    std::vector<double> data_t;

    // Generate the time vector
    int num_samples = static_cast<int>(T * fs);
    std::vector<double> t(num_samples);
    double dt = T / num_samples;
    for (int i = 0; i < num_samples; ++i) {
        t[i] = i * dt;
    }

    // Generate the chirp signal
    for (int rep = 0; rep < num_repetitions; ++rep) {
        for (int i = 0; i < num_samples; ++i) {
            double chirp_signal2 = cos(2 * M_PI * (f_start * t[i] + 0.5 * B * t[i] * t[i] / T));
            data_t.push_back(chirp_signal2);
        }
    }

    size_t length = data_r.size();
    kiss_fft_cpx in[length];
    kiss_fft_cpx out[length];
    kiss_fft_cfg cfg = kiss_fft_alloc(length/4, 0, nullptr, nullptr);

    // Initialize input array
    for (size_t i = 0; i < length; ++i) {
        in[i].r = data_r[i]*data_t[i] * getHanningCoef(length,i);
//        in[i].r = data_r[i];
        in[i].i = 0.0;
    }

    // Perform FFT
    kiss_fft(cfg, in, out);

    // Calculate magnitude of FFT
    std::vector<float> fft_result(length / 2 + 1);
    for (size_t i = 0; i < length / 2 + 1; ++i) {
        fft_result[i] = sqrt(out[i].r * out[i].r + out[i].i * out[i].i);
    }

    // Free memory
    free(cfg);

    return fft_result;
}

std::vector<int> find_peak(const std::vector<float>& fft) {
//    std::vector<int> peak_indices;
//    bool is_above_threshold = false;
//    float threshold = 200; // To be defined based on the fft data
//    int min_distance = 300; // Default value, to be adjusted if needed
//    int last_crossing_index = -1;
//
//    // Determine the threshold, could be a function of the fft data, like its mean, median, etc.
//    threshold = /* some calculation based on fft data */;
//
//    for (int i = 0; i < fft.size(); ++i) {
//        if (!is_above_threshold && fft[i] > threshold) {
//            // Signal goes above the threshold
//            is_above_threshold = true;
//            last_crossing_index = i;
//        } else if (is_above_threshold && fft[i] < threshold) {
//            // Signal goes below the threshold, and we have moved at least min_distance from the last crossing
//            if (i - last_crossing_index > min_distance) {
//                // Use the provided function to find the max index in the range
//                int peak_index = findMaxArrayIdx(const_cast<float*>(&fft[0]), last_crossing_index, i);
//                peak_indices.push_back(peak_index);
//            }
//            is_above_threshold = false;
//        }
//    }
//
//    return peak_indices;
    std::vector<int> peak_indices;
    int max_idx = findMaxArrayIdx(const_cast<float*>(&fft[0]), 0 , fft.size());
    peak_indices.push_back(max_idx);
    float therhold = fft[max_idx]/4;
    int right_max_idx,left_idx;
    right_max_idx = findClosestIdxInArray(const_cast<float*>(&fft[0]), therhold, max_idx, fft.size());
    left_idx = findClosestIdxInArray(const_cast<float*>(&fft[0]), therhold, right_max_idx+500, fft.size());
    int max_idx2 = findMaxArrayIdx(const_cast<float*>(&fft[0]), left_idx , fft.size());
    peak_indices.push_back(max_idx2);
    return peak_indices;
}


bool lab5PitchShift(float *bufferIn) {
    // Lab 4 code is condensed into this function
    int periodLen = detectBufferPeriod(bufferIn);
    float freq = ((float) F_S) / periodLen;

    // If voiced
    if (periodLen > 0) {

        LOGD("Frequency detected: %f\r\n", freq);

        // Epoch detection - this code is written for you, but the principles will be quizzed
        std::vector<int> epochLocations;
        findEpochLocations(epochLocations, bufferIn, periodLen);

        // In this section, you will implement the algorithm given in:
        // https://courses.engr.illinois.edu/ece420/lab5/lab/#buffer-manipulation-algorithm
        //
        // Don't forget about the following functions! API given on the course page.
        //
        // getHanningCoef();
        // findClosestInVector();
        // overlapAndAdd();
        // *********************** START YOUR CODE HERE  **************************** //








        // ************************ END YOUR CODE HERE  ***************************** //
    }

    // Final bookkeeping, move your new pointer back, because you'll be
    // shifting everything back now in your circular buffer
    newEpochIdx -= FRAME_SIZE;
    if (newEpochIdx < FRAME_SIZE) {
        newEpochIdx = FRAME_SIZE;
    }

    return (periodLen > 0);
}

void ece420ProcessFrame(sample_buf *dataBuf) {
    // Keep in mind, we only have 20ms to process each buffer!
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);

    // Get the new desired frequency from android
    FREQ_NEW = FREQ_NEW_ANDROID;

    // Data is encoded in signed PCM-16, little-endian, mono
    int16_t data[FRAME_SIZE];
    for (int i = 0; i < FRAME_SIZE; i++) {
        data[i] = ((uint16_t) dataBuf->buf_[2 * i]) | (((uint16_t) dataBuf->buf_[2 * i + 1]) << 8);
    }

    // Shift our old data back to make room for the new data
    for (int i = 0; i < 2 * FRAME_SIZE; i++) {
        bufferIn[i] = bufferIn[i + FRAME_SIZE - 1];
    }

    // Finally, put in our new data.
    for (int i = 0; i < FRAME_SIZE; i++) {
        bufferIn[i + 2 * FRAME_SIZE - 1] = (float) data[i];
    }

    // The whole kit and kaboodle -- pitch shift
    bool isVoiced = lab5PitchShift(bufferIn);

    if (isVoiced) {
        for (int i = 0; i < FRAME_SIZE; i++) {
            int16_t newVal = (int16_t) bufferOut[i];

            uint8_t lowByte = (uint8_t) (0x00ff & newVal);
            uint8_t highByte = (uint8_t) ((0xff00 & newVal) >> 8);
            dataBuf->buf_[i * 2] = lowByte;
            dataBuf->buf_[i * 2 + 1] = highByte;
        }
    }

    // Very last thing, update your output circular buffer!
    for (int i = 0; i < 2 * FRAME_SIZE; i++) {
        bufferOut[i] = bufferOut[i + FRAME_SIZE - 1];
    }

    for (int i = 0; i < FRAME_SIZE; i++) {
        bufferOut[i + 2 * FRAME_SIZE - 1] = 0;
    }

    gettimeofday(&end, NULL);
    LOGD("Time delay: %ld us",  ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)));
}

// Returns lag l that maximizes sum(x[n] x[n-k])
int detectBufferPeriod(float *buffer) {

    float totalPower = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        totalPower += buffer[i] * buffer[i];
    }

    if (totalPower < VOICED_THRESHOLD) {
        return -1;
    }

    // FFT is done using Kiss FFT engine. Remember to free(cfg) on completion
    kiss_fft_cfg cfg = kiss_fft_alloc(BUFFER_SIZE, false, 0, 0);

    kiss_fft_cpx buffer_in[BUFFER_SIZE];
    kiss_fft_cpx buffer_fft[BUFFER_SIZE];

    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer_in[i].r = bufferIn[i];
        buffer_in[i].i = 0;
    }

    kiss_fft(cfg, buffer_in, buffer_fft);
    free(cfg);


    // Autocorrelation is given by:
    // autoc = ifft(fft(x) * conj(fft(x))
    //
    // Also, (a + jb) (a - jb) = a^2 + b^2
    kiss_fft_cfg cfg_ifft = kiss_fft_alloc(BUFFER_SIZE, true, 0, 0);

    kiss_fft_cpx multiplied_fft[BUFFER_SIZE];
    kiss_fft_cpx autoc_kiss[BUFFER_SIZE];

    for (int i = 0; i < BUFFER_SIZE; i++) {
        multiplied_fft[i].r = (buffer_fft[i].r * buffer_fft[i].r)
                              + (buffer_fft[i].i * buffer_fft[i].i);
        multiplied_fft[i].i = 0;
    }

    kiss_fft(cfg_ifft, multiplied_fft, autoc_kiss);
    free(cfg_ifft);

    // Move to a normal float array rather than a struct array of r/i components
    float autoc[BUFFER_SIZE];
    for (int i = 0; i < BUFFER_SIZE; i++) {
        autoc[i] = autoc_kiss[i].r;
    }

    // We're only interested in pitches below 1000Hz.
    // Why does this line guarantee we only identify pitches below 1000Hz?
    int minIdx = F_S / 1000;
    int maxIdx = BUFFER_SIZE / 2;

    int periodLen = findMaxArrayIdx(autoc, minIdx, maxIdx);
    float freq = ((float) F_S) / periodLen;

    // TODO: tune
    if (freq < 50) {
        periodLen = -1;
    }

    return periodLen;
}


void findEpochLocations(std::vector<int> &epochLocations, float *buffer, int periodLen) {
    // This algorithm requires that the epoch locations be pretty well marked

    int largestPeak = findMaxArrayIdx(bufferIn, 0, BUFFER_SIZE);
    epochLocations.push_back(largestPeak);

    // First go right
    int epochCandidateIdx = epochLocations[0] + periodLen;
    while (epochCandidateIdx < BUFFER_SIZE) {
        epochLocations.push_back(epochCandidateIdx);
        epochCandidateIdx += periodLen;
    }

    // Then go left
    epochCandidateIdx = epochLocations[0] - periodLen;
    while (epochCandidateIdx > 0) {
        epochLocations.push_back(epochCandidateIdx);
        epochCandidateIdx -= periodLen;
    }

    // Sort in place so that we can more easily find the period,
    // where period = (epochLocations[t+1] + epochLocations[t-1]) / 2
    std::sort(epochLocations.begin(), epochLocations.end());

    // Finally, just to make sure we have our epochs in the right
    // place, ensure that every epoch mark (sans first/last) sits on a peak
    for (int i = 1; i < epochLocations.size() - 1; i++) {
        int minIdx = epochLocations[i] - EPOCH_PEAK_REGION_WIGGLE;
        int maxIdx = epochLocations[i] + EPOCH_PEAK_REGION_WIGGLE;

        int peakOffset = findMaxArrayIdx(bufferIn, minIdx, maxIdx) - minIdx;
        peakOffset -= EPOCH_PEAK_REGION_WIGGLE;

        epochLocations[i] += peakOffset;
    }
}

void overlapAddArray(float *dest, float *src, int startIdx, int len) {
    int idxLow = startIdx;
    int idxHigh = startIdx + len;

    int padLow = 0;
    int padHigh = 0;
    if (idxLow < 0) {
        padLow = -idxLow;
    }
    if (idxHigh > BUFFER_SIZE) {
        padHigh = BUFFER_SIZE - idxHigh;
    }

    // Finally, reconstruct the buffer
    for (int i = padLow; i < len + padHigh; i++) {
        dest[startIdx + i] += src[i];
    }
}


