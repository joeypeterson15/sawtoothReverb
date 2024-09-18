#include <iostream>
#include <cmath>
#include <vector>
#include "AudioFile.h"

const int sampleRate = 44100;
const int bitDepth = 16;
const int harmonic = 20;
const float f4Hz = 349.23;
const int aNote = 440; //hertz
const int duration = 1; // seconds

class SawTooth {
    public:
        float f;
        float angle;
        float amplitude;
        float offset;
    SawTooth(float amp, float freq) {
        f = freq;
        amplitude = amp;
        angle = 0.0f;
        offset = 2 * M_PI * f / sampleRate;
    }

    float process(int harmonic) {
        float sample = 0.0f;
        for (int n = 1; n <= harmonic; n++) {
            sample += ((pow(-1, n + 1)) / n) * sin(angle * n);
        }
        angle += offset;
        return (2 / M_PI) * sample * amplitude;
    }
};

// y[n] = x[n] + g·y[n−M] = x[n] + feedbackCoefficient·y[n - delay]
// delay = sampleRate / Fr(resonantFrequency) where Fr = 440 for this project(arbitrary pick)
std::vector<float> combFilter(std::vector<float> x, float feedbackCoefficient, int delay, int N) {
    std::vector<float> y(N, 0.0f);
    for (int i = 0; i < delay; i++) {
        y[i] = x[i];
    }
    for (int i = delay; i < N; i++) {
        y[i] = x[i] + feedbackCoefficient * y[i - delay];
    }

    return y;
}

// y[n] = (−g·x[n]) + x[n−M] + (g·y[n−M])
// y[n]=−a⋅x[n]+x[n−1]+a⋅y[n−1]
std::vector<float> allPassFilter(std::vector<float> x, float filterCoefficient, int delay, int N) {
    std::vector<float> y(N, 0.0f);
    for (int i = 0; i < delay; i++) {
        y[i] = x[i];
    }

    for (int i = delay; i < N; i++) {
        y[i] = (-filterCoefficient * x[i]) + x[i - delay] + (filterCoefficient * y[i - delay]);
    }

    return y;
}

int main() {
    int N = sampleRate * duration;

    SawTooth sawToothA(0.5f, aNote);
    SawTooth sawToothF(0.5f, f4Hz);

    // collect samples from sawTooth
    std::vector<float> audioSamples(N, 0.0f);
    for (int i = 0; i < N; i++) {
        audioSamples[i] = sawToothA.process(harmonic) + sawToothF.process(harmonic);
    }

    // Comb filter -> delay and gain values are taken from stanford text
    std::vector<float> combFilterSamples1 = combFilter(audioSamples, 0.742f, 4799, N);
    std::vector<float> combFilterSamples2 = combFilter(audioSamples, 0.733f, 4999, N);
    std::vector<float> combFilterSamples3 = combFilter(audioSamples, 0.715f, 5399, N);
    std::vector<float> combFilterSamples4 = combFilter(audioSamples, 0.697f, 5801, N);

    // process all the comb filters together(parallel)
    std::vector<float> parallelCombFilterSamples(N);
    for (int i = 0; i < N; i++) {
        parallelCombFilterSamples[i] = combFilterSamples1[i] + combFilterSamples2[i] + combFilterSamples3[i] + combFilterSamples4[i];
    }

    // perform all pass filters(in series)
    std::vector<float> allPassSamples1 = allPassFilter(parallelCombFilterSamples, 0.136f, 900, N);
    std::vector<float> allPassSamples2 = allPassFilter(allPassSamples1, 0.136f, 900, N);

    // NORMALIZE DATA (don't want amplitudes to exceed 1 or -1)
        // a. find the max amplitude
        // b. divide all values by the max amplitude so they go below 1(if the max amplitude is above 1)
    float maxAbsAmp = 0.0f;
    for (int i = 0; i < N; i++) {
        if (fabs(allPassSamples2[i] > maxAbsAmp)) {
            maxAbsAmp = fabs(allPassSamples2[i]);
        }
    }

    if (maxAbsAmp > 1.0f) {
        for (int i = 0; i < N; i++) {
            allPassSamples2[i] = allPassSamples2[i] / maxAbsAmp;
        }
    }


    // write data to WavFile with AudioFile
    AudioFile<float> audioFile;
    audioFile.setNumChannels(1);
    audioFile.setNumSamplesPerChannel(N);
    audioFile.setBitDepth(bitDepth);
    audioFile.setSampleRate(sampleRate);

    for(int i = 0; i < N; i++) {
        audioFile.samples[0][i] = allPassSamples2[i];
        if (i < 150) {
            std::cout << allPassSamples2[i] << std::endl;
        }
    }

    audioFile.save("/Users/joeypeterson/Desktop/sawtooth_with_basic_reverb/sawtoothTest.wav");

    return 0;
}