/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef MODULES_AUDIO_DEVICE_INCLUDE_TEST_AUDIO_DEVICE_H_
#define MODULES_AUDIO_DEVICE_INCLUDE_TEST_AUDIO_DEVICE_H_

#include <memory>
#include <string>
#include <vector>

#include "modules/audio_device/include/audio_device.h"
#include "rtc_base/buffer.h"
#include "rtc_base/event.h"
#include "typedefs.h"  // NOLINT(build/include)

namespace webrtc {

// TestAudioDeviceModule implements an AudioDevice module that can act both as a
// capturer and a renderer. It will use 10ms audio frames.
class TestAudioDeviceModule : public AudioDeviceModule {
 public:
  // Returns the number of samples that Capturers and Renderers with this
  // sampling frequency will work with every time Capture or Render is called.
  static size_t SamplesPerFrame(int sampling_frequency_in_hz);

  class Capturer {
   public:
    virtual ~Capturer() {}
    // Returns the sampling frequency in Hz of the audio data that this
    // capturer produces.
    virtual int SamplingFrequency() const = 0;
    // Replaces the contents of |buffer| with 10ms of captured audio data
    // (see TestAudioDeviceModule::SamplesPerFrame). Returns true if the
    // capturer can keep producing data, or false when the capture finishes.
    virtual bool Capture(rtc::BufferT<int16_t>* buffer) = 0;
  };

  class Renderer {
   public:
    virtual ~Renderer() {}
    // Returns the sampling frequency in Hz of the audio data that this
    // renderer receives.
    virtual int SamplingFrequency() const = 0;
    // Renders the passed audio data and returns true if the renderer wants
    // to keep receiving data, or false otherwise.
    virtual bool Render(rtc::ArrayView<const int16_t> data) = 0;
  };

  // A fake capturer that generates pulses with random samples between
  // -max_amplitude and +max_amplitude.
  class PulsedNoiseCapturer : public Capturer {
   public:
    virtual ~PulsedNoiseCapturer() {}

    virtual void SetMaxAmplitude(int16_t amplitude) = 0;
  };

  virtual ~TestAudioDeviceModule() {}

  // Creates a new TestAudioDeviceModule. When capturing or playing, 10 ms audio
  // frames will be processed every 10ms / |speed|.
  // |capturer| is an object that produces audio data. Can be nullptr if this
  // device is never used for recording.
  // |renderer| is an object that receives audio data that would have been
  // played out. Can be nullptr if this device is never used for playing.
  // Use one of the Create... functions to get these instances.
  static rtc::scoped_refptr<TestAudioDeviceModule> CreateTestAudioDeviceModule(
      std::unique_ptr<Capturer> capturer,
      std::unique_ptr<Renderer> renderer,
      float speed = 1);

  // Returns a Capturer instance that generates a signal where every second
  // frame is zero and every second frame is evenly distributed random noise
  // with max amplitude |max_amplitude|.
  static std::unique_ptr<PulsedNoiseCapturer> CreatePulsedNoiseCapturer(
      int16_t max_amplitude,
      int sampling_frequency_in_hz);

  // Returns a Capturer instance that gets its data from a file.
  static std::unique_ptr<Capturer> CreateWavFileReader(
      std::string filename,
      int sampling_frequency_in_hz);

  // Returns a Capturer instance that gets its data from a file.
  // Automatically detects sample rate.
  static std::unique_ptr<Capturer> CreateWavFileReader(std::string filename);

  // Returns a Renderer instance that writes its data to a file.
  static std::unique_ptr<Renderer> CreateWavFileWriter(
      std::string filename,
      int sampling_frequency_in_hz);

  // Returns a Renderer instance that writes its data to a WAV file, cutting
  // off silence at the beginning (not necessarily perfect silence, see
  // kAmplitudeThreshold) and at the end (only actual 0 samples in this case).
  static std::unique_ptr<Renderer> CreateBoundedWavFileWriter(
      std::string filename,
      int sampling_frequency_in_hz);

  // Returns a Renderer instance that does nothing with the audio data.
  static std::unique_ptr<Renderer> CreateDiscardRenderer(
      int sampling_frequency_in_hz);

  virtual int32_t Init() = 0;
  virtual int32_t RegisterAudioCallback(AudioTransport* callback) = 0;

  virtual int32_t StartPlayout() = 0;
  virtual int32_t StopPlayout() = 0;
  virtual int32_t StartRecording() = 0;
  virtual int32_t StopRecording() = 0;

  virtual bool Playing() const = 0;
  virtual bool Recording() const = 0;

  // Blocks until the Renderer refuses to receive data.
  // Returns false if |timeout_ms| passes before that happens.
  virtual bool WaitForPlayoutEnd(int timeout_ms = rtc::Event::kForever) = 0;
  // Blocks until the Recorder stops producing data.
  // Returns false if |timeout_ms| passes before that happens.
  virtual bool WaitForRecordingEnd(int timeout_ms = rtc::Event::kForever) = 0;
};

}  // namespace webrtc

#endif  // MODULES_AUDIO_DEVICE_INCLUDE_TEST_AUDIO_DEVICE_H_
