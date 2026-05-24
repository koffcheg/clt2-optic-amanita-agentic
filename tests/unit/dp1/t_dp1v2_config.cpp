#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>

#include <jansson.h>

#include "dp1v2/config/config.hpp"

namespace {

struct JsonDeleter {
    void operator()(json_t* value) const
    {
        json_decref(value);
    }
};

using JsonPtr = std::unique_ptr<json_t, JsonDeleter>;

JsonPtr parseJson(const char* text)
{
    json_error_t error{};
    JsonPtr root(json_loads(text, 0, &error));
    if (!root) {
        throw std::runtime_error(error.text);
    }
    return root;
}

JsonPtr makeApplicationConfig()
{
    return parseJson(R"json(
{
  "schema_version": "1.0",
  "source": {
    "mode": "file",
    "file": {
      "path": "unit/input_%06d.tiff",
      "recursive": false,
      "repeat": false
    }
  },
  "logging": {
    "enabled": true,
    "config_file": "config/datapro1_v2-log.xml"
  },
  "profiling": {
    "emit_reports": true,
    "aggregation_window_frames": 300,
    "reports": {
      "emit_frame_reports": false,
      "emit_window_summary": true,
      "emit_run_summary": true
    }
  },
  "dp2": {
    "enabled": false,
    "mode": "disabled",
    "host": "127.0.0.1",
    "port": 11511,
    "reconnect_interval_s": 3
  }
}
)json");
}

JsonPtr makeDisabledApplicationConfig()
{
    return parseJson(R"json(
{
  "schema_version": "1.0",
  "source": {
    "mode": "file",
    "file": {
      "path": "unit/input_%06d.tiff",
      "recursive": false,
      "repeat": false
    }
  },
  "logging": {
    "enabled": false,
    "config_file": "config/datapro1_v2-log.xml"
  },
  "profiling": {
    "emit_reports": true,
    "aggregation_window_frames": 300,
    "reports": {
      "emit_frame_reports": false,
      "emit_window_summary": true,
      "emit_run_summary": true
    }
  },
  "dp2": {
    "enabled": false,
    "mode": "disabled",
    "host": "127.0.0.1",
    "port": 11511,
    "reconnect_interval_s": 3
  }
}
)json");
}

JsonPtr makePipelineConfig()
{
    return parseJson(R"json(
{
  "schema_version": "1.0",
  "profile": "RT-20",
  "input_route": {
    "pixel_format": "U16",
    "bit_depth": 16,
    "pixel_range": {
      "min_value": 0,
      "max_value": 65535,
      "black_level": 0,
      "saturation_level": 65535
    }
  },
  "pipeline": {
    "input_normalization": {
      "enabled": true,
      "variant": "passthrough",
      "level": "L0",
      "parameters": {}
    },
    "prep": {
      "enabled": true,
      "variant": "full_frame",
      "level": "L0",
      "parameters": {}
    },
    "radiometric": {
      "enabled": true,
      "variant": "inverse_median",
      "level": "L1",
      "parameters": {
        "inverse_median": {
          "enabled": true,
          "mode": "FixedK3",
          "stride": 1,
          "output_median_frame": false,
          "output_dynamic_range_mode": "RawSigned"
        }
      }
    },
    "enhancement": {
      "enabled": false,
      "variant": "gaussian",
      "level": "L0",
      "parameters": {}
    },
    "matched_filter": {
      "enabled": false,
      "variant": "gaussian",
      "level": "L0",
      "parameters": {}
    },
    "candidate_extraction": {
      "enabled": false,
      "variant": "global_threshold",
      "level": "L0",
      "parameters": {}
    },
    "segmentation": {
      "enabled": false,
      "variant": "single_morphology",
      "level": "L0",
      "parameters": {}
    },
    "object_filtering": {
      "enabled": false,
      "variant": "area",
      "level": "L0",
      "parameters": {}
    },
    "measurement": {
      "enabled": false,
      "variant": "centroid_bbox",
      "level": "L0",
      "parameters": {}
    }
  }
}
)json");
}

json_t* objectAt(json_t* root, std::initializer_list<const char*> path)
{
    json_t* current = root;
    for (const char* key : path) {
        current = json_object_get(current, key);
        if (!json_is_object(current)) {
            throw std::runtime_error(std::string("missing object in test JSON: ") + key);
        }
    }
    return current;
}

void removeKey(json_t* root, std::initializer_list<const char*> object_path, const char* key)
{
    ASSERT_EQ(json_object_del(objectAt(root, object_path), key), 0);
}

void setString(json_t* root, std::initializer_list<const char*> object_path, const char* key, const char* value)
{
    ASSERT_EQ(json_object_set_new(objectAt(root, object_path), key, json_string(value)), 0);
}

void setBool(json_t* root, std::initializer_list<const char*> object_path, const char* key, bool value)
{
    ASSERT_EQ(json_object_set_new(objectAt(root, object_path), key, json_boolean(value)), 0);
}

void setInteger(json_t* root, std::initializer_list<const char*> object_path, const char* key, json_int_t value)
{
    ASSERT_EQ(json_object_set_new(objectAt(root, object_path), key, json_integer(value)), 0);
}

void setReal(json_t* root, std::initializer_list<const char*> object_path, const char* key, double value)
{
    ASSERT_EQ(json_object_set_new(objectAt(root, object_path), key, json_real(value)), 0);
}

void setObject(json_t* root, std::initializer_list<const char*> object_path, const char* key, json_t* value)
{
    ASSERT_EQ(json_object_set_new(objectAt(root, object_path), key, value), 0);
}

json_t* makePrepTilesParameters(
    const json_int_t tile_width,
    const json_int_t tile_height,
    const json_int_t overlap_x,
    const json_int_t overlap_y)
{
    json_t* tiles = json_object();
    EXPECT_EQ(json_object_set_new(tiles, "tile_width", json_integer(tile_width)), 0);
    EXPECT_EQ(json_object_set_new(tiles, "tile_height", json_integer(tile_height)), 0);
    EXPECT_EQ(json_object_set_new(tiles, "overlap_x", json_integer(overlap_x)), 0);
    EXPECT_EQ(json_object_set_new(tiles, "overlap_y", json_integer(overlap_y)), 0);

    json_t* parameters = json_object();
    EXPECT_EQ(json_object_set_new(parameters, "tiles", tiles), 0);
    return parameters;
}

class ConfigTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
        temp_dir_ = std::filesystem::temp_directory_path() /
                    (std::string("dp1v2_config_test_") + std::to_string(timestamp));
        std::filesystem::create_directories(temp_dir_);
    }

    void TearDown() override
    {
        std::error_code ec;
        std::filesystem::remove_all(temp_dir_, ec);
    }

    std::filesystem::path writeJson(json_t* root, const std::string& name)
    {
        const std::filesystem::path path = temp_dir_ / name;
        if (json_dump_file(root, path.string().c_str(), JSON_INDENT(2)) != 0) {
            throw std::runtime_error("failed to write test JSON");
        }
        return path;
    }

    dp1v2::ApplicationConfig loadApplication(JsonPtr& application)
    {
        return dp1v2::load_application_config(writeJson(application.get(), "application.json").string());
    }

    dp1v2::PipelineConfig loadPipeline(JsonPtr& pipeline)
    {
        return dp1v2::load_pipeline_config(writeJson(pipeline.get(), "pipeline.json").string());
    }

    dp1v2::Dp1Config loadDp1(JsonPtr& application, JsonPtr& pipeline)
    {
        return dp1v2::load_dp1_config(writeJson(application.get(), "application.json").string(),
                                      writeJson(pipeline.get(), "pipeline.json").string());
    }

private:
    std::filesystem::path temp_dir_;
};

}  // namespace

TEST_F(ConfigTest, LoadDp1Config_WhenCanonicalApplicationAndPipelineFilesAreValid_ReturnsTypedDp1Config)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();

    const dp1v2::Dp1Config config = loadDp1(application, pipeline);

    EXPECT_EQ(config.application.schema_version, "1.0");
    EXPECT_EQ(config.application.source.mode, dp1v2::FrameSourceMode::File);
    EXPECT_EQ(config.application.source.file.path, "unit/input_%06d.tiff");
    EXPECT_EQ(config.application.dp2.mode, dp1v2::DP2ConnectionMode::Disabled);
    EXPECT_EQ(config.pipeline.schema_version, "1.0");
    EXPECT_EQ(config.pipeline.input_route.pixel_format, dp1v2::PixelFormat::U16);
    EXPECT_EQ(config.pipeline.input_route.bit_depth, dp1v2::InputBitDepth::Bit16);
    EXPECT_TRUE(config.pipeline.stages.input_normalization.enabled);
    EXPECT_EQ(config.pipeline.stages.input_normalization.variant, "passthrough");
    EXPECT_EQ(config.pipeline.stages.radiometric.variant, "inverse_median");
    EXPECT_TRUE(config.pipeline.stages.radiometric.parameters.contains("inverse_median"));
    ASSERT_TRUE(config.resolved_pipeline.radiometric.inverse_median.has_value());
    EXPECT_EQ(config.resolved_pipeline.radiometric.inverse_median->mode, dp1v2::InverseMedianMode::FixedK3);
    EXPECT_EQ(config.resolved_pipeline.radiometric.inverse_median->stride, 1);
    EXPECT_EQ(config.resolved_pipeline.radiometric.inverse_median->output_dynamic_range_mode,
              dp1v2::InverseMedianOutputMode::RawSigned);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenMinimalLoggingProfilingConfigIsValid_AppliesInternalDefaults)
{
    JsonPtr application = makeApplicationConfig();

    const dp1v2::ApplicationConfig config = loadApplication(application);

    EXPECT_TRUE(config.logging.enabled);
    EXPECT_EQ(config.logging.realtime_profile, "rt_safe");
    EXPECT_TRUE(config.logging.structured_messages);
    EXPECT_TRUE(config.logging.sanitize_external_strings);
    EXPECT_EQ(config.logging.max_field_length, 256);
    EXPECT_EQ(config.logging.max_messages_per_frame, 64);
    EXPECT_EQ(config.logging.max_messages_per_tile, 16);
    EXPECT_TRUE(config.logging.mdc.enabled);
    EXPECT_TRUE(config.logging.mdc.fields.empty());
    EXPECT_EQ(config.logging.sampling.frame_summary_every_n, 100);
    EXPECT_EQ(config.logging.sampling.rate_limit_per_event_per_sec, 1);
    EXPECT_TRUE(config.logging.sampling.duplicate_suppression);
    EXPECT_TRUE(config.logging.async.enabled);
    EXPECT_EQ(config.logging.async.buffer_size, 1024);
    EXPECT_FALSE(config.logging.async.blocking);
    EXPECT_EQ(config.logging.async.discard_policy, "drop_debug_and_summarize");
    EXPECT_TRUE(config.logging.logger_overrides.empty());

    EXPECT_TRUE(config.profiling.emit_reports);
    EXPECT_EQ(config.profiling.mode, "lightweight");
    ASSERT_EQ(config.profiling.levels.size(), 5U);
    EXPECT_EQ(config.profiling.levels[0], "P0");
    EXPECT_EQ(config.profiling.levels[1], "P1");
    EXPECT_EQ(config.profiling.levels[2], "P2");
    EXPECT_EQ(config.profiling.levels[3], "P4");
    EXPECT_EQ(config.profiling.levels[4], "P5");
    EXPECT_FALSE(config.profiling.raw_trace.enabled);
    EXPECT_EQ(config.profiling.raw_trace.max_frames, 0);
    EXPECT_EQ(config.profiling.raw_trace.max_events_per_frame, 64);
    EXPECT_FALSE(config.profiling.operation_timing.enabled);
    EXPECT_TRUE(config.profiling.operation_timing.include_format_conversions);
    EXPECT_TRUE(config.profiling.operation_timing.include_memory_copies);
    EXPECT_FALSE(config.profiling.operation_timing.include_allocations);
    EXPECT_EQ(config.profiling.reports.format, "json");
    EXPECT_TRUE(config.profiling.reports.output_dir.empty());
    EXPECT_TRUE(config.profiling.logging_bridge.emit_aggregated_summaries);
    EXPECT_EQ(config.profiling.logging_bridge.summary_every_n_frames, 300);
    EXPECT_FALSE(config.profiling.logging_bridge.emit_budget_warnings);
    EXPECT_FALSE(config.profiling.external_trace.enabled);
    EXPECT_EQ(config.profiling.external_trace.backend, "none");
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenMinimalClientLoggingProfilingSurfaceHasNoHiddenFields_Parses)
{
    JsonPtr application = makeApplicationConfig();

    EXPECT_EQ(json_object_get(objectAt(application.get(), {"logging"}), "default_level"), nullptr);
    EXPECT_EQ(json_object_get(objectAt(application.get(), {"logging"}), "realtime_profile"), nullptr);
    EXPECT_EQ(json_object_get(objectAt(application.get(), {"logging"}), "structured_messages"), nullptr);
    EXPECT_EQ(json_object_get(objectAt(application.get(), {"logging"}), "sanitize_external_strings"), nullptr);
    EXPECT_EQ(json_object_get(objectAt(application.get(), {"logging"}), "max_field_length"), nullptr);
    EXPECT_EQ(json_object_get(objectAt(application.get(), {"logging"}), "max_messages_per_frame"), nullptr);
    EXPECT_EQ(json_object_get(objectAt(application.get(), {"logging"}), "max_messages_per_tile"), nullptr);
    EXPECT_EQ(json_object_get(objectAt(application.get(), {"logging"}), "mdc"), nullptr);
    EXPECT_EQ(json_object_get(objectAt(application.get(), {"logging"}), "sampling"), nullptr);
    EXPECT_EQ(json_object_get(objectAt(application.get(), {"logging"}), "async"), nullptr);
    EXPECT_EQ(json_object_get(objectAt(application.get(), {"logging"}), "logger_overrides"), nullptr);
    EXPECT_EQ(json_object_get(objectAt(application.get(), {"profiling"}), "enabled"), nullptr);
    EXPECT_EQ(json_object_get(objectAt(application.get(), {"profiling"}), "mode"), nullptr);
    EXPECT_EQ(json_object_get(objectAt(application.get(), {"profiling"}), "levels"), nullptr);
    EXPECT_EQ(json_object_get(objectAt(application.get(), {"profiling"}), "logging_bridge"), nullptr);

    const dp1v2::ApplicationConfig config = loadApplication(application);

    EXPECT_TRUE(config.logging.enabled);
    EXPECT_EQ(config.logging.config_file, "config/datapro1_v2-log.xml");
    EXPECT_TRUE(config.profiling.emit_reports);
    EXPECT_EQ(config.profiling.aggregation_window_frames, 300);
    EXPECT_FALSE(config.profiling.reports.emit_frame_reports);
    EXPECT_TRUE(config.profiling.reports.emit_window_summary);
    EXPECT_TRUE(config.profiling.reports.emit_run_summary);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenDisabledMinimalLoggingConfigIsValid_ReturnsTypedConfig)
{
    JsonPtr application = makeDisabledApplicationConfig();

    const dp1v2::ApplicationConfig config = loadApplication(application);

    EXPECT_FALSE(config.logging.enabled);
    EXPECT_EQ(config.logging.config_file, "config/datapro1_v2-log.xml");
    EXPECT_FALSE(config.profiling.logging_bridge.emit_budget_warnings);
    EXPECT_FALSE(config.profiling.external_trace.enabled);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenLoggingDefaultLevelExists_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    setString(application.get(), {"logging"}, "default_level", "INFO");

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenLoggingRealtimeProfileExists_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    setString(application.get(), {"logging"}, "realtime_profile", "rt_safe");

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenLoggingStructuredMessagesExists_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    setBool(application.get(), {"logging"}, "structured_messages", true);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenLoggingSanitizeExternalStringsExists_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    setBool(application.get(), {"logging"}, "sanitize_external_strings", true);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenLoggingMaxFieldLengthExists_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    setInteger(application.get(), {"logging"}, "max_field_length", 512);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenLoggingMaxMessagesPerFrameExists_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    setInteger(application.get(), {"logging"}, "max_messages_per_frame", 64);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenLoggingMaxMessagesPerTileExists_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    setInteger(application.get(), {"logging"}, "max_messages_per_tile", 16);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenLoggingMdcExists_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* mdc = json_object();
    ASSERT_EQ(json_object_set_new(mdc, "enabled", json_boolean(true)), 0);
    setObject(application.get(), {"logging"}, "mdc", mdc);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenLoggingSamplingExists_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* sampling = json_object();
    ASSERT_EQ(json_object_set_new(sampling, "frame_summary_every_n", json_integer(100)), 0);
    setObject(application.get(), {"logging"}, "sampling", sampling);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenLoggingAsyncExists_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* async = json_object();
    ASSERT_EQ(json_object_set_new(async, "enabled", json_boolean(true)), 0);
    setObject(application.get(), {"logging"}, "async", async);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenLoggingLoggerOverridesExists_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* overrides = json_array();
    setObject(application.get(), {"logging"}, "logger_overrides", overrides);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenProfilingRawTraceExists_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* raw_trace = json_object();
    ASSERT_EQ(json_object_set_new(raw_trace, "enabled", json_boolean(false)), 0);
    setObject(application.get(), {"profiling"}, "raw_trace", raw_trace);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenProfilingOperationTimingExists_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* operation_timing = json_object();
    ASSERT_EQ(json_object_set_new(operation_timing, "enabled", json_boolean(false)), 0);
    setObject(application.get(), {"profiling"}, "operation_timing", operation_timing);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenProfilingModeExists_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    setString(application.get(), {"profiling"}, "mode", "lightweight");

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenProfilingLevelsExists_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* levels = json_array();
    ASSERT_EQ(json_array_append_new(levels, json_string("P1")), 0);
    setObject(application.get(), {"profiling"}, "levels", levels);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenProfilingLoggingBridgeExists_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* logging_bridge = json_object();
    ASSERT_EQ(json_object_set_new(logging_bridge, "emit_aggregated_summaries", json_boolean(true)), 0);
    setObject(application.get(), {"profiling"}, "logging_bridge", logging_bridge);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenProfilingExternalTraceExists_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* external_trace = json_object();
    ASSERT_EQ(json_object_set_new(external_trace, "enabled", json_boolean(false)), 0);
    setObject(application.get(), {"profiling"}, "external_trace", external_trace);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenProfilingReportsFormatExists_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    setString(application.get(), {"profiling", "reports"}, "format", "json");

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenProfilingReportsOutputDirExists_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    setString(application.get(), {"profiling", "reports"}, "output_dir", "profile_reports");

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenLoggingHasUnknownKey_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    setInteger(application.get(), {"logging"}, "unknown", 1);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenProfilingHasUnknownKey_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    setInteger(application.get(), {"profiling"}, "unknown", 1);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenProfilingEnabledLegacyKeyExists_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    setBool(application.get(), {"profiling"}, "enabled", true);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenPipelineWrapperExists_ThrowsConfigError)
{
    JsonPtr application = parseJson(R"json({"pipeline": {}})json");

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadPipelineConfig_WhenApplicationWrapperExists_ThrowsConfigError)
{
    JsonPtr pipeline = parseJson(R"json({"application": {}})json");

    EXPECT_THROW(loadPipeline(pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenApplicationSchemaVersionMissing_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    removeKey(application.get(), {}, "schema_version");

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenPipelineSchemaVersionUnsupported_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setString(pipeline.get(), {}, "schema_version", "2.0");

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenLegacyRootConfigExists_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setObject(application.get(), {}, "config", json_object());

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenLegacyDp2ConnExists_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setObject(application.get(), {}, "dp2conn", json_object());

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenSourceModeIsFileAndPathExistsInConfig_ReturnsFileSourceConfig)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setString(application.get(), {"source", "file"}, "path", "relative/source.tiff");

    const dp1v2::Dp1Config config = loadDp1(application, pipeline);

    EXPECT_EQ(config.application.source.mode, dp1v2::FrameSourceMode::File);
    EXPECT_EQ(config.application.source.file.path, "relative/source.tiff");
}

TEST_F(ConfigTest, LoadDp1Config_WhenSourceModeIsCameraPro_ThrowsForFirstIteration)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setString(application.get(), {"source"}, "mode", "camerapro");

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenSourceFilePathMissing_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    removeKey(application.get(), {"source", "file"}, "path");

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenDp2PortIsZero_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setInteger(application.get(), {"dp2"}, "port", 0);

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenDp2ReconnectIntervalExceedsIntRange_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setInteger(application.get(), {"dp2"}, "reconnect_interval_s", 2147483648LL);

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenDp2EnabledWithDisabledMode_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setBool(application.get(), {"dp2"}, "enabled", true);
    setString(application.get(), {"dp2"}, "mode", "disabled");

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenInputRouteIsU16Bit12WithOrderedRange_ReturnsBit12Route)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setString(pipeline.get(), {"input_route"}, "pixel_format", "U16");
    setInteger(pipeline.get(), {"input_route"}, "bit_depth", 12);
    setReal(pipeline.get(), {"input_route", "pixel_range"}, "min_value", 0.0);
    setReal(pipeline.get(), {"input_route", "pixel_range"}, "black_level", 4.0);
    setReal(pipeline.get(), {"input_route", "pixel_range"}, "saturation_level", 4095.0);
    setReal(pipeline.get(), {"input_route", "pixel_range"}, "max_value", 4095.0);

    const dp1v2::Dp1Config config = loadDp1(application, pipeline);

    EXPECT_EQ(config.pipeline.input_route.pixel_format, dp1v2::PixelFormat::U16);
    EXPECT_EQ(config.pipeline.input_route.bit_depth, dp1v2::InputBitDepth::Bit12);
    EXPECT_EQ(config.pipeline.input_route.pixel_range.black_level, 4.0);
    EXPECT_EQ(config.pipeline.input_route.pixel_range.saturation_level, 4095.0);
}

TEST_F(ConfigTest, LoadDp1Config_WhenBitDepthIsString_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setString(pipeline.get(), {"input_route"}, "bit_depth", "12");

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenInputRouteIsU8Bit12_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setString(pipeline.get(), {"input_route"}, "pixel_format", "U8");
    setInteger(pipeline.get(), {"input_route"}, "bit_depth", 12);

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenPixelRangeIsNotOrdered_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setReal(pipeline.get(), {"input_route", "pixel_range"}, "black_level", 500.0);
    setReal(pipeline.get(), {"input_route", "pixel_range"}, "saturation_level", 100.0);

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenRequiredStageKeyIsMissing_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    removeKey(pipeline.get(), {"pipeline"}, "measurement");

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenFullStageInterfaceNameIsUsedAsConfigKey_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    json_t* radiometric = json_deep_copy(objectAt(pipeline.get(), {"pipeline", "radiometric"}));
    removeKey(pipeline.get(), {"pipeline"}, "radiometric");
    setObject(pipeline.get(), {"pipeline"}, "radiometric_correction", radiometric);

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenOldAcquisitionStageKeyIsUsed_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setObject(pipeline.get(), {"pipeline"}, "acquisition", json_object());

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenPipelineConfigCStageKeyRadiometricIsUsed_ReturnsStageConfig)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();

    const dp1v2::Dp1Config config = loadDp1(application, pipeline);

    EXPECT_TRUE(config.pipeline.stages.radiometric.enabled);
    EXPECT_EQ(config.pipeline.stages.radiometric.variant, "inverse_median");
    EXPECT_EQ(config.pipeline.stages.radiometric.level, "L1");
}

TEST_F(ConfigTest, LoadDp1Config_WhenPrepFullFrameParametersAreEmpty_ReturnsNoPrepTilesConfig)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();

    const dp1v2::Dp1Config config = loadDp1(application, pipeline);

    EXPECT_FALSE(config.resolved_pipeline.prep.tiles.has_value());
}

TEST_F(ConfigTest, LoadDp1Config_WhenPrepTilesParametersAreValid_ReturnsTypedPrepTilesConfig)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setString(pipeline.get(), {"pipeline", "prep"}, "variant", "tiles");
    setString(pipeline.get(), {"pipeline", "prep"}, "level", "L1");
    setObject(pipeline.get(), {"pipeline", "prep"}, "parameters", makePrepTilesParameters(256, 256, 16, 16));

    const dp1v2::Dp1Config config = loadDp1(application, pipeline);

    ASSERT_TRUE(config.resolved_pipeline.prep.tiles.has_value());
    EXPECT_EQ(config.resolved_pipeline.prep.tiles->tile_width, 256);
    EXPECT_EQ(config.resolved_pipeline.prep.tiles->tile_height, 256);
    EXPECT_EQ(config.resolved_pipeline.prep.tiles->overlap_x, 16);
    EXPECT_EQ(config.resolved_pipeline.prep.tiles->overlap_y, 16);
}

TEST_F(ConfigTest, LoadDp1Config_WhenPrepTilesObjectIsMissing_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setString(pipeline.get(), {"pipeline", "prep"}, "variant", "tiles");
    setObject(pipeline.get(), {"pipeline", "prep"}, "parameters", json_object());

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenPrepTileWidthIsZero_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setString(pipeline.get(), {"pipeline", "prep"}, "variant", "tiles");
    setObject(pipeline.get(), {"pipeline", "prep"}, "parameters", makePrepTilesParameters(0, 256, 16, 16));

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenPrepTileHeightIsZero_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setString(pipeline.get(), {"pipeline", "prep"}, "variant", "tiles");
    setObject(pipeline.get(), {"pipeline", "prep"}, "parameters", makePrepTilesParameters(256, 0, 16, 16));

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenPrepOverlapXIsNegative_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setString(pipeline.get(), {"pipeline", "prep"}, "variant", "tiles");
    setObject(pipeline.get(), {"pipeline", "prep"}, "parameters", makePrepTilesParameters(256, 256, -1, 16));

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenPrepOverlapYIsNegative_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setString(pipeline.get(), {"pipeline", "prep"}, "variant", "tiles");
    setObject(pipeline.get(), {"pipeline", "prep"}, "parameters", makePrepTilesParameters(256, 256, 16, -1));

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenPrepOverlapXReachesTileWidth_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setString(pipeline.get(), {"pipeline", "prep"}, "variant", "tiles");
    setObject(pipeline.get(), {"pipeline", "prep"}, "parameters", makePrepTilesParameters(256, 256, 256, 16));

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenPrepOverlapYReachesTileHeight_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setString(pipeline.get(), {"pipeline", "prep"}, "variant", "tiles");
    setObject(pipeline.get(), {"pipeline", "prep"}, "parameters", makePrepTilesParameters(256, 256, 16, 256));

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenPrepTilesHasUnknownKey_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setString(pipeline.get(), {"pipeline", "prep"}, "variant", "tiles");
    setObject(pipeline.get(), {"pipeline", "prep"}, "parameters", makePrepTilesParameters(256, 256, 16, 16));
    setInteger(pipeline.get(), {"pipeline", "prep", "parameters", "tiles"}, "border", 8);

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenPrepFullFrameHasUnknownParameter_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setInteger(pipeline.get(), {"pipeline", "prep", "parameters"}, "tile_width", 256);

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenStageVariantIsNotRegistered_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setString(pipeline.get(), {"pipeline", "radiometric"}, "variant", "not_registered");

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenStageLevelIsInvalid_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setString(pipeline.get(), {"pipeline", "radiometric"}, "level", "L9");

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenInverseMedianParametersObjectMissing_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setObject(pipeline.get(), {"pipeline", "radiometric"}, "parameters", json_object());

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenInverseMedianOptionalFieldsAreOmitted_AppliesCanonicalDefaults)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    json_t* parameters = json_object();
    ASSERT_EQ(json_object_set_new(parameters, "inverse_median", json_object()), 0);
    setObject(pipeline.get(), {"pipeline", "radiometric"}, "parameters", parameters);

    const dp1v2::Dp1Config config = loadDp1(application, pipeline);

    ASSERT_TRUE(config.resolved_pipeline.radiometric.inverse_median.has_value());
    const dp1v2::InverseMedianParametersConfig resolved = *config.resolved_pipeline.radiometric.inverse_median;
    EXPECT_TRUE(resolved.enabled);
    EXPECT_EQ(resolved.mode, dp1v2::InverseMedianMode::FixedK3);
    EXPECT_EQ(resolved.stride, 1);
    EXPECT_FALSE(resolved.output_median_frame);
    EXPECT_EQ(resolved.output_dynamic_range_mode, dp1v2::InverseMedianOutputMode::RawSigned);
}

TEST_F(ConfigTest, LoadDp1Config_WhenInverseMedianStrideIsZero_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setInteger(pipeline.get(), {"pipeline", "radiometric", "parameters", "inverse_median"}, "stride", 0);

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenParameterIntegerExceedsIntRange_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setInteger(pipeline.get(), {"pipeline", "radiometric", "parameters", "inverse_median"}, "stride", 2147483648LL);

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenInverseMedianUnknownKeyExists_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setInteger(pipeline.get(), {"pipeline", "radiometric", "parameters", "inverse_median"}, "window", 7);

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenLoggingConfigFileMissing_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    removeKey(application.get(), {"logging"}, "config_file");

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenLoggingConfigFileEmpty_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setString(application.get(), {"logging"}, "config_file", "");

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenVisualizationSectionMissing_AppliesDisabledDefaults)
{
    JsonPtr application = makeApplicationConfig();

    const dp1v2::ApplicationConfig config = loadApplication(application);

    EXPECT_FALSE(config.visualization.enabled);
    EXPECT_EQ(config.visualization.output_dir, "datapro1_v2_output/visualization");
    EXPECT_EQ(config.visualization.mode, "sync_file");
    EXPECT_EQ(config.visualization.every_n_frames, 1);
    EXPECT_EQ(config.visualization.max_frames, 0);
    EXPECT_TRUE(config.visualization.stages.empty());
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenVisualizationSectionIsValid_ReturnsVisualizationConfig)
{
    JsonPtr application = makeApplicationConfig();
    json_t* stages = json_array();
    ASSERT_EQ(json_array_append_new(stages, json_string("radiometric")), 0);
    json_t* visualization = json_object();
    ASSERT_EQ(json_object_set_new(visualization, "enabled", json_boolean(true)), 0);
    ASSERT_EQ(json_object_set_new(visualization, "output_dir", json_string("out/vis")), 0);
    ASSERT_EQ(json_object_set_new(visualization, "mode", json_string("sync_file")), 0);
    ASSERT_EQ(json_object_set_new(visualization, "every_n_frames", json_integer(2)), 0);
    ASSERT_EQ(json_object_set_new(visualization, "max_frames", json_integer(3)), 0);
    ASSERT_EQ(json_object_set_new(visualization, "stages", stages), 0);
    setObject(application.get(), {}, "visualization", visualization);

    const dp1v2::ApplicationConfig config = loadApplication(application);

    EXPECT_TRUE(config.visualization.enabled);
    EXPECT_EQ(config.visualization.output_dir, "out/vis");
    EXPECT_EQ(config.visualization.mode, "sync_file");
    EXPECT_EQ(config.visualization.every_n_frames, 2);
    EXPECT_EQ(config.visualization.max_frames, 3);
    ASSERT_EQ(config.visualization.stages.size(), 1U);
    EXPECT_EQ(config.visualization.stages[0], "radiometric");
}


TEST_F(ConfigTest, LoadApplicationConfig_WhenVisualizationSectionIsNotObject_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    setObject(application.get(), {}, "visualization", json_array());

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenVisualizationSectionIsEmptyObject_AppliesVisualizationDefaults)
{
    JsonPtr application = makeApplicationConfig();
    setObject(application.get(), {}, "visualization", json_object());

    const dp1v2::ApplicationConfig config = loadApplication(application);

    EXPECT_FALSE(config.visualization.enabled);
    EXPECT_EQ(config.visualization.output_dir, "datapro1_v2_output/visualization");
    EXPECT_EQ(config.visualization.mode, "sync_file");
    EXPECT_EQ(config.visualization.every_n_frames, 1);
    EXPECT_EQ(config.visualization.max_frames, 0);
    EXPECT_TRUE(config.visualization.stages.empty());
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenVisualizationEnabledIsNotBool_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* visualization = json_object();
    ASSERT_EQ(json_object_set_new(visualization, "enabled", json_string("true")), 0);
    setObject(application.get(), {}, "visualization", visualization);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenVisualizationOutputDirIsNotString_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* visualization = json_object();
    ASSERT_EQ(json_object_set_new(visualization, "output_dir", json_integer(7)), 0);
    setObject(application.get(), {}, "visualization", visualization);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenVisualizationModeIsNotString_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* visualization = json_object();
    ASSERT_EQ(json_object_set_new(visualization, "mode", json_integer(1)), 0);
    setObject(application.get(), {}, "visualization", visualization);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenVisualizationEveryNFramesIsNotInteger_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* visualization = json_object();
    ASSERT_EQ(json_object_set_new(visualization, "every_n_frames", json_string("1")), 0);
    setObject(application.get(), {}, "visualization", visualization);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenVisualizationMaxFramesIsNotInteger_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* visualization = json_object();
    ASSERT_EQ(json_object_set_new(visualization, "max_frames", json_string("0")), 0);
    setObject(application.get(), {}, "visualization", visualization);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenVisualizationStagesIsNotArray_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* visualization = json_object();
    ASSERT_EQ(json_object_set_new(visualization, "stages", json_string("radiometric")), 0);
    setObject(application.get(), {}, "visualization", visualization);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenVisualizationStageIsNotString_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* stages = json_array();
    ASSERT_EQ(json_array_append_new(stages, json_integer(1)), 0);
    json_t* visualization = json_object();
    ASSERT_EQ(json_object_set_new(visualization, "stages", stages), 0);
    setObject(application.get(), {}, "visualization", visualization);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenVisualizationStageIsEmpty_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* stages = json_array();
    ASSERT_EQ(json_array_append_new(stages, json_string("")), 0);
    json_t* visualization = json_object();
    ASSERT_EQ(json_object_set_new(visualization, "stages", stages), 0);
    setObject(application.get(), {}, "visualization", visualization);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenVisualizationDisabledWithEmptyOutputDir_ReturnsConfig)
{
    JsonPtr application = makeApplicationConfig();
    json_t* visualization = json_object();
    ASSERT_EQ(json_object_set_new(visualization, "enabled", json_boolean(false)), 0);
    ASSERT_EQ(json_object_set_new(visualization, "output_dir", json_string("")), 0);
    setObject(application.get(), {}, "visualization", visualization);

    const dp1v2::ApplicationConfig config = loadApplication(application);

    EXPECT_FALSE(config.visualization.enabled);
    EXPECT_TRUE(config.visualization.output_dir.empty());
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenVisualizationEnabledWithEmptyOutputDir_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* visualization = json_object();
    ASSERT_EQ(json_object_set_new(visualization, "enabled", json_boolean(true)), 0);
    ASSERT_EQ(json_object_set_new(visualization, "output_dir", json_string("")), 0);
    setObject(application.get(), {}, "visualization", visualization);

    try {
        (void)loadApplication(application);
        FAIL() << "expected config error";
    } catch (const std::logic_error& error) {
        const std::string message = error.what();
        EXPECT_NE(message.find("application.visualization.output_dir"), std::string::npos);
        EXPECT_NE(message.find("empty output_dir"), std::string::npos);
    }
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenVisualizationHasUnknownKey_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* visualization = json_object();
    ASSERT_EQ(json_object_set_new(visualization, "enabled", json_boolean(true)), 0);
    ASSERT_EQ(json_object_set_new(visualization, "extra", json_boolean(true)), 0);
    setObject(application.get(), {}, "visualization", visualization);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenVisualizationModeIsUnsupported_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* visualization = json_object();
    ASSERT_EQ(json_object_set_new(visualization, "enabled", json_boolean(true)), 0);
    ASSERT_EQ(json_object_set_new(visualization, "mode", json_string("async")), 0);
    setObject(application.get(), {}, "visualization", visualization);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenVisualizationEveryNFramesIsZero_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* visualization = json_object();
    ASSERT_EQ(json_object_set_new(visualization, "enabled", json_boolean(true)), 0);
    ASSERT_EQ(json_object_set_new(visualization, "every_n_frames", json_integer(0)), 0);
    setObject(application.get(), {}, "visualization", visualization);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenVisualizationMaxFramesIsNegative_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* visualization = json_object();
    ASSERT_EQ(json_object_set_new(visualization, "enabled", json_boolean(true)), 0);
    ASSERT_EQ(json_object_set_new(visualization, "max_frames", json_integer(-1)), 0);
    setObject(application.get(), {}, "visualization", visualization);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenVisualizationStagesContainDuplicate_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* stages = json_array();
    ASSERT_EQ(json_array_append_new(stages, json_string("radiometric")), 0);
    ASSERT_EQ(json_array_append_new(stages, json_string("radiometric")), 0);
    json_t* visualization = json_object();
    ASSERT_EQ(json_object_set_new(visualization, "enabled", json_boolean(true)), 0);
    ASSERT_EQ(json_object_set_new(visualization, "stages", stages), 0);
    setObject(application.get(), {}, "visualization", visualization);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadApplicationConfig_WhenVisualizationStageIsUnsupported_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    json_t* stages = json_array();
    ASSERT_EQ(json_array_append_new(stages, json_string("measurement")), 0);
    json_t* visualization = json_object();
    ASSERT_EQ(json_object_set_new(visualization, "enabled", json_boolean(true)), 0);
    ASSERT_EQ(json_object_set_new(visualization, "stages", stages), 0);
    setObject(application.get(), {}, "visualization", visualization);

    EXPECT_THROW(loadApplication(application), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenInputNormalizationPassthroughDisabledKbin1_Parses)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();

    json_t* binning = json_object();
    ASSERT_EQ(json_object_set_new(binning, "mode", json_string("disabled")), 0);
    ASSERT_EQ(json_object_set_new(binning, "kbin", json_integer(1)), 0);
    json_t* parameters = json_object();
    ASSERT_EQ(json_object_set_new(parameters, "binning", binning), 0);
    setObject(pipeline.get(), {"pipeline", "input_normalization"}, "parameters", parameters);

    const dp1v2::Dp1Config config = loadDp1(application, pipeline);

    EXPECT_EQ(config.pipeline.stages.input_normalization.variant, "passthrough");
    EXPECT_EQ(config.resolved_pipeline.input_normalization.binning_mode, dp1v2::BinningMode::None);
    EXPECT_EQ(config.resolved_pipeline.input_normalization.bin_factor, 1);
}

TEST_F(ConfigTest, LoadDp1Config_WhenInputNormalizationPassthroughAverage_Throws)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();

    json_t* binning = json_object();
    ASSERT_EQ(json_object_set_new(binning, "mode", json_string("average")), 0);
    ASSERT_EQ(json_object_set_new(binning, "kbin", json_integer(2)), 0);
    json_t* parameters = json_object();
    ASSERT_EQ(json_object_set_new(parameters, "binning", binning), 0);
    setObject(pipeline.get(), {"pipeline", "input_normalization"}, "parameters", parameters);

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenInputNormalizationPassthroughDisabledKbinNotOne_Throws)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();

    json_t* binning = json_object();
    ASSERT_EQ(json_object_set_new(binning, "mode", json_string("disabled")), 0);
    ASSERT_EQ(json_object_set_new(binning, "kbin", json_integer(2)), 0);
    json_t* parameters = json_object();
    ASSERT_EQ(json_object_set_new(parameters, "binning", binning), 0);
    setObject(pipeline.get(), {"pipeline", "input_normalization"}, "parameters", parameters);

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenInputNormalizationAverageBinningAverageKbin2Or4_Parses)
{
    for (const int kbin : {2, 4}) {
        JsonPtr application = makeApplicationConfig();
        JsonPtr pipeline = makePipelineConfig();
        setString(pipeline.get(), {"pipeline", "input_normalization"}, "variant", "average_binning");

        json_t* binning = json_object();
        ASSERT_EQ(json_object_set_new(binning, "mode", json_string("average")), 0);
        ASSERT_EQ(json_object_set_new(binning, "kbin", json_integer(kbin)), 0);
        json_t* parameters = json_object();
        ASSERT_EQ(json_object_set_new(parameters, "binning", binning), 0);
        setObject(pipeline.get(), {"pipeline", "input_normalization"}, "parameters", parameters);

        const dp1v2::Dp1Config config = loadDp1(application, pipeline);

        EXPECT_EQ(config.pipeline.stages.input_normalization.variant, "average_binning");
        EXPECT_EQ(config.resolved_pipeline.input_normalization.binning_mode, dp1v2::BinningMode::Average);
        EXPECT_EQ(config.resolved_pipeline.input_normalization.bin_factor, kbin);
    }
}

TEST_F(ConfigTest, LoadDp1Config_WhenInputNormalizationBinningModeUnsupported_Throws)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();

    json_t* binning = json_object();
    ASSERT_EQ(json_object_set_new(binning, "mode", json_string("sum")), 0);
    json_t* parameters = json_object();
    ASSERT_EQ(json_object_set_new(parameters, "binning", binning), 0);
    setObject(pipeline.get(), {"pipeline", "input_normalization"}, "parameters", parameters);

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenInputNormalizationAverageBinningDisabled_Throws)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setString(pipeline.get(), {"pipeline", "input_normalization"}, "variant", "average_binning");

    json_t* binning = json_object();
    ASSERT_EQ(json_object_set_new(binning, "mode", json_string("disabled")), 0);
    ASSERT_EQ(json_object_set_new(binning, "kbin", json_integer(1)), 0);
    json_t* parameters = json_object();
    ASSERT_EQ(json_object_set_new(parameters, "binning", binning), 0);
    setObject(pipeline.get(), {"pipeline", "input_normalization"}, "parameters", parameters);

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenInputNormalizationAverageBinningAverageKbin1_Throws)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setString(pipeline.get(), {"pipeline", "input_normalization"}, "variant", "average_binning");

    json_t* binning = json_object();
    ASSERT_EQ(json_object_set_new(binning, "mode", json_string("average")), 0);
    ASSERT_EQ(json_object_set_new(binning, "kbin", json_integer(1)), 0);
    json_t* parameters = json_object();
    ASSERT_EQ(json_object_set_new(parameters, "binning", binning), 0);
    setObject(pipeline.get(), {"pipeline", "input_normalization"}, "parameters", parameters);

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenInputNormalizationAverageBinningKbinUnsupported_Throws)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setString(pipeline.get(), {"pipeline", "input_normalization"}, "variant", "average_binning");

    json_t* binning = json_object();
    ASSERT_EQ(json_object_set_new(binning, "mode", json_string("average")), 0);
    ASSERT_EQ(json_object_set_new(binning, "kbin", json_integer(3)), 0);
    json_t* parameters = json_object();
    ASSERT_EQ(json_object_set_new(parameters, "binning", binning), 0);
    setObject(pipeline.get(), {"pipeline", "input_normalization"}, "parameters", parameters);

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenInputNormalizationBinningHasUnknownKey_Throws)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setString(pipeline.get(), {"pipeline", "input_normalization"}, "variant", "average_binning");

    json_t* binning = json_object();
    ASSERT_EQ(json_object_set_new(binning, "mode", json_string("average")), 0);
    ASSERT_EQ(json_object_set_new(binning, "kbin", json_integer(2)), 0);
    ASSERT_EQ(json_object_set_new(binning, "extra", json_integer(1)), 0);
    json_t* parameters = json_object();
    ASSERT_EQ(json_object_set_new(parameters, "binning", binning), 0);
    setObject(pipeline.get(), {"pipeline", "input_normalization"}, "parameters", parameters);

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}
