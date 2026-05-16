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
    "enabled": false,
    "config_file": "config/datapro1_v2-log.xml",
    "default_level": "INFO",
    "realtime_profile": "rt_safe",
    "structured_messages": true,
    "sanitize_external_strings": true,
    "max_field_length": 256,
    "max_messages_per_frame": 64,
    "max_messages_per_tile": 16,
    "mdc": {
      "enabled": true,
      "fields": ["pipeline_run_id", "camera_id", "source_id", "frame_id", "stage"]
    },
    "sampling": {
      "frame_summary_every_n": 100,
      "rate_limit_per_event_per_sec": 1,
      "duplicate_suppression": true
    },
    "async": {
      "enabled": true,
      "buffer_size": 1024,
      "blocking": false,
      "discard_policy": "drop_debug_and_summarize"
    },
    "logger_overrides": []
  },
  "profiling": {
    "enabled": true,
    "mode": "lightweight",
    "levels": ["P0", "P1", "P2", "P4", "P5"],
    "aggregation_window_frames": 300,
    "raw_trace": {
      "enabled": false,
      "max_frames": 0,
      "max_events_per_frame": 64
    },
    "operation_timing": {
      "enabled": false,
      "include_format_conversions": true,
      "include_memory_copies": true,
      "include_allocations": false
    },
    "reports": {
      "emit_frame_reports": false,
      "emit_window_summary": true,
      "emit_run_summary": true,
      "format": "json",
      "output_dir": ""
    },
    "logging_bridge": {
      "emit_aggregated_summaries": true,
      "summary_every_n_frames": 300,
      "emit_budget_warnings": true
    },
    "external_trace": {
      "enabled": false,
      "backend": "none"
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
    EXPECT_EQ(config.pipeline.stages.radiometric.variant, "inverse_median");
    EXPECT_TRUE(config.pipeline.stages.radiometric.parameters.contains("inverse_median"));
    ASSERT_TRUE(config.resolved_pipeline.radiometric.inverse_median.has_value());
    EXPECT_EQ(config.resolved_pipeline.radiometric.inverse_median->mode, dp1v2::InverseMedianMode::FixedK3);
    EXPECT_EQ(config.resolved_pipeline.radiometric.inverse_median->stride, 1);
    EXPECT_EQ(config.resolved_pipeline.radiometric.inverse_median->output_dynamic_range_mode,
              dp1v2::InverseMedianOutputMode::RawSigned);
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

TEST_F(ConfigTest, LoadDp1Config_WhenIntegerFieldExceedsIntRange_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setInteger(application.get(), {"logging"}, "max_field_length", 2147483648LL);

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

TEST_F(ConfigTest, LoadDp1Config_WhenPipelineConfigCStageKeyRadiometricIsUsed_ReturnsStageConfig)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();

    const dp1v2::Dp1Config config = loadDp1(application, pipeline);

    EXPECT_TRUE(config.pipeline.stages.radiometric.enabled);
    EXPECT_EQ(config.pipeline.stages.radiometric.variant, "inverse_median");
    EXPECT_EQ(config.pipeline.stages.radiometric.level, "L1");
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

TEST_F(ConfigTest, LoadDp1Config_WhenProfilingLevelIsDuplicated_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    json_t* levels = json_array();
    ASSERT_EQ(json_array_append_new(levels, json_string("P1")), 0);
    ASSERT_EQ(json_array_append_new(levels, json_string("P1")), 0);
    setObject(application.get(), {"profiling"}, "levels", levels);

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenExternalTraceIsEnabled_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setBool(application.get(), {"profiling", "external_trace"}, "enabled", true);

    EXPECT_THROW(loadDp1(application, pipeline), std::logic_error);
}

TEST_F(ConfigTest, LoadDp1Config_WhenLoggingAsyncBlocksRtSafeProfile_ThrowsConfigError)
{
    JsonPtr application = makeApplicationConfig();
    JsonPtr pipeline = makePipelineConfig();
    setBool(application.get(), {"logging", "async"}, "blocking", true);

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
