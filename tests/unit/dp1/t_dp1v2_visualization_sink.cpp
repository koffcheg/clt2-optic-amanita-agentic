#include <gtest/gtest.h>

#include <chrono>
#include <cstdio>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include <opencv2/imgcodecs.hpp>

#include "dp1v2/visualization/visualization_sink.hpp"

namespace {

class VisualizationSinkTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        const auto unique = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        temp_dir_ = std::filesystem::temp_directory_path() / ("dp1v2_visualization_sink_test_" + unique);
        std::filesystem::create_directories(temp_dir_);
    }

    void TearDown() override
    {
        std::error_code ec;
        std::filesystem::remove_all(temp_dir_, ec);
    }

    dp1v2::VisualizationConfig enabledConfig() const
    {
        dp1v2::VisualizationConfig config{};
        config.enabled = true;
        config.output_dir = temp_dir_.string();
        config.mode = "sync_file";
        config.every_n_frames = 1;
        config.max_frames = 0;
        config.stages = {"radiometric"};
        return config;
    }

    dp1v2::FrameContext frameContext(std::uint64_t frame_id = 0) const
    {
        dp1v2::FrameContext context{};
        context.frame_id = frame_id;
        context.camera_id = 7;
        return context;
    }

    std::filesystem::path manifestPath(std::uint64_t frame_id = 0) const
    {
        char buffer[32]{};
        std::snprintf(buffer, sizeof(buffer), "frame_%06llu", static_cast<unsigned long long>(frame_id));
        return temp_dir_ / buffer / "radiometric.json";
    }

    std::filesystem::path pngPath(std::uint64_t frame_id = 0) const
    {
        char buffer[32]{};
        std::snprintf(buffer, sizeof(buffer), "frame_%06llu", static_cast<unsigned long long>(frame_id));
        return temp_dir_ / buffer / "radiometric_frame.png";
    }

    static dp1v2::StageOutcome<dp1v2::RadiometricFullFrameOutput> skippedOutcome()
    {
        return dp1v2::StageOutcome<dp1v2::RadiometricFullFrameOutput>{
            .status = dp1v2::StageExecutionStatus::Skipped,
            .output = {},
            .reason = "warming up",
        };
    }

    static dp1v2::StageOutcome<dp1v2::RadiometricFullFrameOutput> completedOutcome()
    {
        cv::Mat residual(2, 2, CV_16SC1);
        residual.at<std::int16_t>(0, 0) = -10;
        residual.at<std::int16_t>(0, 1) = 0;
        residual.at<std::int16_t>(1, 0) = 25;
        residual.at<std::int16_t>(1, 1) = 100;

        dp1v2::ProcessingFrame frame{};
        frame.frame_id = 0;
        frame.image = residual;
        frame.pixel_format = dp1v2::PixelFormat::S16;
        frame.value_range = dp1v2::PixelRange{.min_value = -255.0, .max_value = 255.0, .black_level = 0.0, .saturation_level = 255.0};
        frame.processing_domain = dp1v2::ProcessingDomain::RadiometricResidual;
        frame.range_policy = dp1v2::RangePolicy::SignedResidual;
        frame.geometry = dp1v2::FrameGeometry{.width = residual.cols, .height = residual.rows};

        return dp1v2::StageOutcome<dp1v2::RadiometricFullFrameOutput>{
            .status = dp1v2::StageExecutionStatus::Completed,
            .output = dp1v2::RadiometricFullFrameOutput{.frame = frame},
            .reason = "",
        };
    }

    static std::string readText(const std::filesystem::path& path)
    {
        std::ifstream input(path);
        return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    }

    std::filesystem::path temp_dir_;
};

} // namespace

TEST_F(VisualizationSinkTest, WriteStageOutput_WhenDisabled_CreatesNoOutput)
{
    dp1v2::VisualizationConfig config = enabledConfig();
    config.enabled = false;
    dp1v2::VisualizationSink sink(config);

    sink.write_stage_output(frameContext(), "radiometric", completedOutcome());

    EXPECT_FALSE(std::filesystem::exists(manifestPath()));
    EXPECT_FALSE(std::filesystem::exists(pngPath()));
}

TEST_F(VisualizationSinkTest, WriteStageOutput_WhenStageNotListed_CreatesNoOutput)
{
    dp1v2::VisualizationConfig config = enabledConfig();
    config.stages = {"other"};
    dp1v2::VisualizationSink sink(config);

    sink.write_stage_output(frameContext(), "radiometric", completedOutcome());

    EXPECT_FALSE(std::filesystem::exists(manifestPath()));
}

TEST_F(VisualizationSinkTest, WriteStageOutput_WhenSkipped_WritesManifestOnly)
{
    dp1v2::VisualizationSink sink(enabledConfig());

    sink.write_stage_output(frameContext(), "radiometric", skippedOutcome());

    ASSERT_TRUE(std::filesystem::exists(manifestPath()));
    EXPECT_FALSE(std::filesystem::exists(pngPath()));
    const std::string manifest = readText(manifestPath());
    EXPECT_NE(manifest.find("\"status\": \"skipped\""), std::string::npos);
    EXPECT_NE(manifest.find("\"outputs\": [  ]"), std::string::npos);
}

TEST_F(VisualizationSinkTest, WriteStageOutput_WhenCompletedSignedResidual_WritesManifestAndPng)
{
    dp1v2::VisualizationSink sink(enabledConfig());

    sink.write_stage_output(frameContext(), "radiometric", completedOutcome());

    ASSERT_TRUE(std::filesystem::exists(manifestPath()));
    ASSERT_TRUE(std::filesystem::exists(pngPath()));
    const cv::Mat preview = cv::imread(pngPath().string(), cv::IMREAD_UNCHANGED);
    ASSERT_FALSE(preview.empty());
    EXPECT_EQ(preview.type(), CV_8UC1);
    const std::string manifest = readText(manifestPath());
    EXPECT_NE(manifest.find("\"renderer\": \"clip_to_input_range_png_preview\""), std::string::npos);
    EXPECT_NE(manifest.find("\"range_policy\": \"SignedResidual\""), std::string::npos);
}

TEST_F(VisualizationSinkTest, WriteStageOutput_WhenEveryNFramesDoesNotMatch_CreatesNoOutput)
{
    dp1v2::VisualizationConfig config = enabledConfig();
    config.every_n_frames = 2;
    dp1v2::VisualizationSink sink(config);

    sink.write_stage_output(frameContext(1), "radiometric", completedOutcome());

    EXPECT_FALSE(std::filesystem::exists(manifestPath(1)));
}

TEST_F(VisualizationSinkTest, WriteStageOutput_WhenMaxFramesReached_SkipsFurtherFrames)
{
    dp1v2::VisualizationConfig config = enabledConfig();
    config.max_frames = 1;
    dp1v2::VisualizationSink sink(config);

    sink.write_stage_output(frameContext(0), "radiometric", skippedOutcome());
    sink.write_stage_output(frameContext(1), "radiometric", skippedOutcome());

    EXPECT_TRUE(std::filesystem::exists(manifestPath(0)));
    EXPECT_FALSE(std::filesystem::exists(manifestPath(1)));
}

TEST_F(VisualizationSinkTest, WriteStageOutput_WhenFailed_WritesManifestOnly)
{
    dp1v2::VisualizationSink sink(enabledConfig());
    const dp1v2::StageOutcome<dp1v2::RadiometricFullFrameOutput> failed{
        .status = dp1v2::StageExecutionStatus::Failed,
        .output = {},
        .reason = "stage failed",
    };

    sink.write_stage_output(frameContext(), "radiometric", failed);

    ASSERT_TRUE(std::filesystem::exists(manifestPath()));
    EXPECT_FALSE(std::filesystem::exists(pngPath()));
    const std::string manifest = readText(manifestPath());
    EXPECT_NE(manifest.find("\"status\": \"failed\""), std::string::npos);
    EXPECT_NE(manifest.find("stage failed"), std::string::npos);
}
