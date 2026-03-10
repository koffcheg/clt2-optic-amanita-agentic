
#pragma once

#include <chrono>
#include <string>
#include <vector>

struct TimeDataRow {
  std::chrono::system_clock::time_point t;
  std::vector<double> measures;
};

class FileReader {
public:
  FileReader();

  void loadFile(const std::string &filename);
  void clear();

  const std::vector<std::string> &columns() { return m_columns; };
  const std::vector<TimeDataRow> &data() { return m_data; };

private:
  TimeDataRow decodeDataLine(const std::string& line);

  char m_delimeter = ' ';
  std::string m_dateTimeFormat="%Y.%m.%d %H:%M:%S";

  std::vector<std::string> m_columns;
  std::vector<TimeDataRow> m_data;
  size_t m_readBytes = 0;
};

std::vector<std::string> split(const std::string &str, char delimiter);
