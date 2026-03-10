
#include "webcamera.hpp"
#include "utils.hpp"
#include <fmt/format.h>
#include <jansson.h>
#include <logHelper.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdexcept>

namespace cam_pro {

#define FOURCC_YUYV 0x56595559
#define FOURCC_MJPG 0x47504A4D

static auto logger = log4cxx::Logger::getLogger("camera.web");

const std::string WebCamera::type("webcam");

const std::vector<std::pair<int, const char *>> WebCamera::m_props = {
    {cv::CAP_PROP_FRAME_WIDTH, "width"},
    {cv::CAP_PROP_FRAME_HEIGHT, "height"},
    {cv::CAP_PROP_FPS, "fps"},
    {cv::CAP_PROP_FORMAT, "format"},
    {cv::CAP_PROP_MODE, "mode"},
    {cv::CAP_PROP_BRIGHTNESS, "brightness"},
    {cv::CAP_PROP_CONTRAST, "contrast"},
    {cv::CAP_PROP_SATURATION, "saturation"},
    {cv::CAP_PROP_HUE, "hue"},
    {cv::CAP_PROP_GAIN, "gain"},
    {cv::CAP_PROP_EXPOSURE, "exposure"},
    {cv::CAP_PROP_AUTO_EXPOSURE, "autoExposure"},
    {cv::CAP_PROP_GAMMA, "gamma"},
    {cv::CAP_PROP_TEMPERATURE, "t"},
    {cv::CAP_PROP_ZOOM, "zoom"},
    {cv::CAP_PROP_FOCUS, "focus"},
    {cv::CAP_PROP_AUTOFOCUS, "autofocus"},
    {cv::CAP_PROP_BUFFERSIZE, "buffer"},
    {cv::CAP_PROP_IRIS, "iris"},
    {cv::CAP_PROP_CODEC_PIXEL_FORMAT, "codecPixelFormat"},
    {cv::CAP_PROP_READ_TIMEOUT_MSEC, "readTimeoutms"}
    };

void WebCamera::loadConfig(const json_t *config) {

  if (!config) {
    // todo: log
    throw std::runtime_error("Invalid webcam configuration");
  }

  jansson_cfg_obj_reader reader(config, type.c_str());

  m_id = reader.read_int_param("id");

  // doesn't it worth to move some options to the Camera class?
  m_desiredFps = reader.read_double_param("fps", m_desiredFps);
  m_temporalBinning =
      reader.read_bool_param("temporalBinning", m_temporalBinning);

  m_grayScale.customGray =
      reader.read_bool_param("grayScale.customGray", m_grayScale.customGray);
  m_grayScale.r = reader.read_double_param("grayScale.r", m_grayScale.r);
  m_grayScale.g = reader.read_double_param("grayScale.g", m_grayScale.g);
  m_grayScale.b = reader.read_double_param("grayScale.b", m_grayScale.b);

  // autoscale
  float s = (m_grayScale.r + m_grayScale.g + m_grayScale.b);
  s = s > 0 ? 256 / s : 256;
  s = reader.read_double_param("grayScale.scale", s);

  m_grayScale.scale = s;

  LOG_INFO(fmt::format("customGray={}, rgb=[{}, {}, {}], scale={}",
                       m_grayScale.customGray, m_grayScale.r, m_grayScale.g,
                       m_grayScale.b, m_grayScale.scale));

  m_flipV = reader.read_bool_param("flipV", m_flipV);
  m_flipH = reader.read_bool_param("flipH", m_flipH);

  for (auto &p : m_props) {
    double v = reader.read_double_param(p.second, NAN);
    if (!isnan(v)) {
      m_config.props.push_back({p.first, v});
    }
  }

  std::string fourcc = reader.read_string_param("fourcc", "");
  if (fourcc.length() == 4) {
    uint64_t v64 = 0;
    for (size_t i = 0; i < fourcc.length(); i++) {
      v64 += fourcc[i] * (1ull << i * 8);
    }
    m_config.props.push_back({cv::CAP_PROP_FOURCC, v64});
  }
}

WebCamera::WebCamera(const json_t *config) : Camera(config) {
  loadConfig(config);
}

WebCamera::~WebCamera() {}

void WebCamera::printProperties(const char *prompt) {
  std::string s(prompt);
  for (auto &p : m_props) {
    s += fmt::format("{}={}, ", p.second, m_cap->get(p.first));
  }
  s += fmt::format("{}={}", "fourcc", m_cap->get(cv::CAP_PROP_FOURCC));
  LOG_INFO(s);
}

void WebCamera::open() {
  // Declaring an object to capture stream of frames from default camera
  m_cap = std::make_unique<cv::VideoCapture>(m_id);
  m_opened = m_cap->isOpened();
  if (!m_cap->isOpened()) {
    LOG_ERROR("Can not open the camera");
    return;
  }

  printProperties("Default properties: ");
  // set capture properties
  for (auto &p : m_config.props) {
    m_cap->set(p.first, p.second);
  }
  printProperties("Final properties: ");

  m_width = m_cap->get(cv::CAP_PROP_FRAME_WIDTH);
  m_height = m_cap->get(cv::CAP_PROP_FRAME_HEIGHT);
  m_bytesPerPixel = 2; // todo:
  m_rawFps = m_cap->get(cv::CAP_PROP_FPS);
  m_divider = m_desiredFps > 0 ? lround(m_rawFps / m_desiredFps) : 1;
  m_divider = m_divider > 0 ? m_divider : 1;
  m_fps = m_rawFps / m_divider;
  m_depth = 8; // ceil(log2(256 * m_divider));
  LOG_INFO(fmt::format("webcam {} is opened. {}x{} rawFps={}, fps={}", m_id,
                       m_width, m_height, m_rawFps, m_fps));
}

void WebCamera::close() {
  m_cap->release(); // Releasing the buffer memory//
}

void WebCamera::start(){};
void WebCamera::stop(){};

bool WebCamera::saveImage(const cv::Mat &mat, void *buffer, int bufferSize) {
  if (mat.empty()) {
    return false;
  }
  int correctBufferSize = m_width * m_height * 2;
  if (bufferSize != correctBufferSize) { // todo:
    LOG_ERROR(fmt::format("Invalid buffer size: {} instead of {}", bufferSize,
                          correctBufferSize));
    throw std::runtime_error("Invalid buffer size");
  }
  if (mat.type() != CV_16U) {
    LOG_ERROR(fmt::format("Invalid buffer size: {} instead of {}", bufferSize,
                          correctBufferSize));
    throw std::runtime_error("Invalid matrix format");
  }
  short *p = (short *)buffer;
  std::memcpy(p, mat.data, m_width * m_height * sizeof(short));
  return true;
}

void WebCamera::customGray(const cv::Mat &src, cv::Mat &dst) {
  // Check if the source image is valid
  if (src.empty() || src.type() != CV_8UC3) {
    LOG_ERROR("Invalid input image");
    return;
  }

  if (dst.size() != src.size() || dst.type() != CV_16UC1) {
    // Create a destination image of the same size but 16-bit single channel
    // (grayscale)
    dst = cv::Mat::zeros(src.size(), CV_16UC1);
  }

  // Loop over each pixel and apply the custom weights for B, G, and R channels
  for (int y = 0; y < src.rows; ++y) {
    for (int x = 0; x < src.cols; ++x) {
      // Convert to grayscale using the custom weights
      auto bgrPixel = src.at<cv::Vec3b>(y, x);

      float wsum = bgrPixel[0] * m_grayScale.b + // Blue channel
                   bgrPixel[1] * m_grayScale.g + // Green channel
                   bgrPixel[2] * m_grayScale.r;  // Red channel

      wsum = u::coerce(wsum * m_grayScale.scale, 0.0f, 65535.0f);

      uint16_t grayValue = static_cast<uint16_t>(wsum);

      // Set the computed value in the destination image
      dst.at<uint16_t>(y, x) = grayValue;
    }
  }
}

bool WebCamera::getImage(void *buffer, int bufferSize) {
  if (m_cap) {
    LOG_TRACE("Getting image");
    *m_cap.get() >> m_img;
    m_msFromLastFrame = m_ticTac.tac<std::milli>();
    m_ticTac.tic();
    if (m_img.empty()) {
      LOG_ERROR("Invalid input image. Camera disconnected");
      m_opened = false;
      return false;
    }

    LOG_TRACE("New webcam image");
    if (m_grayScale.customGray) {
      customGray(m_img, m_imgGray16);
    } else {
      cv::cvtColor(m_img, m_imgGray, cv::COLOR_BGR2GRAY);
      m_imgGray.convertTo(m_imgGray16, CV_16UC1, 256, 0);
    }
    //   fliping
    if (m_flipV || m_flipH) {
      int code = 0;
      if (m_flipV && m_flipH) {
        code = -1;
      } else if (m_flipV && !m_flipH) {
        code = 0;
      } else if (!m_flipV && m_flipH) {
        code = 1;
      };
      cv::Mat dst; // dst must be a different Mat
      cv::flip(m_imgGray16, dst, code);
      m_imgGray16 = dst;
    }

    m_dividerCounter++;
    if (m_dividerCounter >= m_divider) {
      // temporal decimation / binning
      m_dividerCounter = 0;

      if (m_temporalBinning && m_divider) {
        m_imgGray16 = m_imgBinning;
        // m_imgBinning.convertTo(m_imgGray16, CV_16UC1, 1.0, 0);
      }
      return saveImage(m_imgGray16, buffer, bufferSize);
    } else {
      if (m_temporalBinning) {
        if (m_imgBinning.empty() || m_dividerCounter == 1) {
          m_imgBinning = m_imgGray16;
        } else {
          m_imgBinning += m_imgGray16;
        }
      }
    }
  }
  return false;
};

} // namespace cam_pro