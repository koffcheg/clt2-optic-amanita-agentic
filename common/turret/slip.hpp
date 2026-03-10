
#pragma once

#include <functional>
#include <vector>

// todo: unit tests

namespace slip {

using Data = std::vector<unsigned char>;
using SlipDecoderCallback = std::function<void(const Data &decodedData)>;

class Decoder {
public:
  explicit Decoder(SlipDecoderCallback onFrame);

  void decode(unsigned char byte);
  void decode(const void *data, size_t size);
  void decode(const Data &data);

  void reset();

  int frameErrorCount() { return m_frameErrorCount; }
private:
  void processFrame();

  Data m_decoded;
  SlipDecoderCallback m_onFrame;
  bool m_esc;
  int m_frameErrorCount;
};

void encode(const void *inputFrame, size_t inputSize, Data &output);

/// @brief Encodes Frame (input data vector) into
/// @param input
/// @param output
void encode(const Data &inputFrame, Data &output);

} // namespace slip