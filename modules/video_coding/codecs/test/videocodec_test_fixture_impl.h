/*
 *  Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_VIDEO_CODING_CODECS_TEST_VIDEOCODEC_TEST_FIXTURE_IMPL_H_
#define MODULES_VIDEO_CODING_CODECS_TEST_VIDEOCODEC_TEST_FIXTURE_IMPL_H_

#include <memory>
#include <string>
#include <vector>

#include "api/test/videocodec_test_fixture.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "api/video_codecs/video_encoder_factory.h"
#include "common_types.h"  // NOLINT(build/include)
#include "common_video/h264/h264_common.h"
#include "modules/video_coding/codecs/test/stats.h"
#include "modules/video_coding/codecs/test/test_config.h"
#include "modules/video_coding/codecs/test/videoprocessor.h"
#include "modules/video_coding/utility/ivf_file_writer.h"
#include "rtc_base/task_queue_for_test.h"
#include "test/testsupport/frame_reader.h"
#include "test/testsupport/frame_writer.h"

namespace webrtc {
namespace test {

// Integration test for video processor. It does rate control and frame quality
// analysis using frame statistics collected by video processor and logs the
// results. If thresholds are specified it checks that corresponding metrics
// are in desirable range.
class VideoCodecTestFixtureImpl : public VideoCodecTestFixture {
  // Verifies that all H.264 keyframes contain SPS/PPS/IDR NALUs.
 public:
  class H264KeyframeChecker : public TestConfig::EncodedFrameChecker {
   public:
    void CheckEncodedFrame(webrtc::VideoCodecType codec,
                           const EncodedImage& encoded_frame) const override;
  };

  explicit VideoCodecTestFixtureImpl(TestConfig config);
  VideoCodecTestFixtureImpl(
      TestConfig config,
      std::unique_ptr<VideoDecoderFactory> decoder_factory,
      std::unique_ptr<VideoEncoderFactory> encoder_factory);
  ~VideoCodecTestFixtureImpl() override;

  void RunTest(const std::vector<RateProfile>& rate_profiles,
               const std::vector<RateControlThresholds>* rc_thresholds,
               const std::vector<QualityThresholds>* quality_thresholds,
               const BitstreamThresholds* bs_thresholds,
               const VisualizationParams* visualization_params) override;

  Stats GetStats() override;

 private:
  class CpuProcessTime;

  void CreateEncoderAndDecoder();
  void DestroyEncoderAndDecoder();
  void SetUpAndInitObjects(rtc::test::TaskQueueForTest* task_queue,
                           int initial_bitrate_kbps,
                           int initial_framerate_fps,
                           const VisualizationParams* visualization_params);
  void ReleaseAndCloseObjects(rtc::test::TaskQueueForTest* task_queue);

  void ProcessAllFrames(rtc::TaskQueue* task_queue,
                        const std::vector<RateProfile>& rate_profiles);
  void AnalyzeAllFrames(
      const std::vector<RateProfile>& rate_profiles,
      const std::vector<RateControlThresholds>* rc_thresholds,
      const std::vector<QualityThresholds>* quality_thresholds,
      const BitstreamThresholds* bs_thresholds);

  void VerifyVideoStatistic(const VideoStatistics& video_stat,
                            const RateControlThresholds* rc_thresholds,
                            const QualityThresholds* quality_thresholds,
                            const BitstreamThresholds* bs_thresholds,
                            size_t target_bitrate_kbps,
                            float input_framerate_fps);

  void PrintSettings(rtc::test::TaskQueueForTest* task_queue) const;
  std::unique_ptr<VideoDecoderFactory> CreateDecoderFactory();
  std::unique_ptr<VideoEncoderFactory> CreateEncoderFactory();

  // Codecs.
  std::unique_ptr<VideoDecoderFactory> decoder_factory_;
  std::unique_ptr<VideoEncoderFactory> encoder_factory_;
  std::unique_ptr<VideoEncoder> encoder_;
  VideoProcessor::VideoDecoderList decoders_;

  // Helper objects.
  TestConfig config_;
  Stats stats_;
  std::unique_ptr<FrameReader> source_frame_reader_;
  VideoProcessor::IvfFileWriterList encoded_frame_writers_;
  VideoProcessor::FrameWriterList decoded_frame_writers_;
  std::unique_ptr<VideoProcessor> processor_;
  std::unique_ptr<CpuProcessTime> cpu_process_time_;
};

}  // namespace test
}  // namespace webrtc

#endif  // MODULES_VIDEO_CODING_CODECS_TEST_VIDEOCODEC_TEST_FIXTURE_IMPL_H_
