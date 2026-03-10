

#include "slip.hpp"
#include <vector>

namespace slip {

#define END 0xC0     // indicates end of packet
#define ESC 0xDB     // indicates byte stuffing
#define ESC_END 0xDC // ESC ESC_END means END data byte
#define ESC_ESC 0xDD // ESC ESC_ESC means ESC data byte

Decoder::Decoder(SlipDecoderCallback onFrame)
    : m_onFrame(onFrame), m_esc(false), m_frameErrorCount(0) {}

void Decoder::processFrame() {
  if (!m_decoded.empty()) {
    m_onFrame(m_decoded);
    m_decoded.clear();
  }
}

void Decoder::decode(unsigned char byte) {
  if (byte == END) { // frame end received
    processFrame();
    m_esc = false;
    return;
  }

  if (m_esc) {
    if (byte == ESC_END) {
      m_decoded.push_back(END);
    } else if (byte == ESC_ESC) {
      m_decoded.push_back(ESC);
    } else {
      m_frameErrorCount++;
    }
    m_esc = false;
  } else {
    if (byte == ESC) {
      m_esc = true;
    } else {
      m_decoded.push_back(byte);
    }
  }
}

void Decoder::decode(const void *data, size_t size) {
  const unsigned char *p = static_cast<const unsigned char *>(data);
  for (size_t i = 0; i < size; i++) {
    decode(*p++);
  }
}

void Decoder::decode(const Data &data) {
  for (auto byte : data) {
    decode(byte);
  }
}

void Decoder::reset() {
  m_esc = false;
  m_frameErrorCount = 0;
  m_decoded.clear();
}

void encode(const void *inputFrame, size_t inputSize, Data &output) {
  const unsigned char *p = static_cast<const unsigned char *>(inputFrame);
  for (size_t i = 0; i < inputSize; i++, p++) {
    if (*p == END) {
      output.push_back(ESC);
      output.push_back(ESC_END);
    } else if (*p == ESC) {
      output.push_back(ESC);
      output.push_back(ESC_ESC);
    } else {
      output.push_back(*p);
    }
  }
  output.push_back(END);
}

void encode(const Data &inputFrame, Data &output) {
  if (inputFrame.empty()) {
    encode(nullptr, 0, output);
    return;
  }

  encode(inputFrame.data(), inputFrame.size(), output);
}

} // namespace slip