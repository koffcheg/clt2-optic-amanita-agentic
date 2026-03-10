

#include "fileReader.hpp"

#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

std::vector<std::string> split(const std::string &str, char delimiter) {
  std::vector<std::string> words;
  size_t start = 0;
  size_t end = str.find(delimiter);

  while (end != std::string::npos) {
    std::string ss = str.substr(start, end - start);
    if (!ss.empty()) {
      words.push_back(ss); // Extract each word
    }
    start = end + 1;                  // Move past the delimiter
    end = str.find(delimiter, start); // Find the next delimiter
  }
  std::string ss = str.substr(start);
  if (!ss.empty()) {
    words.push_back(ss); // Add the last word
  }
  return words;
}

bool parseTime(const std::string &time_str, const std::string &format,
               std::chrono::system_clock::time_point &t) {
  size_t dot_pos = time_str.rfind('.');

  std::string time_part;
  std::string subsecond_part;

  if (dot_pos > 10) { //10=length("2024.04.10")
    time_part = time_str.substr(0, dot_pos); // Time part without subseconds
    subsecond_part = "0" + (dot_pos != std::string::npos && dot_pos > 10)
                         ? time_str.substr(dot_pos + 1)
                         : "0"; // Subseconds part
  } else {
    time_part = time_str;
  }

  std::tm tm = {};
  std::istringstream ss(time_str);

  // parse the time string according to the format
  ss >> std::get_time(&tm, format.c_str());
  if (ss.fail()) {
    return false;
  }

  // Convert std::tm to time_t (calendar time in seconds since epoch)
  std::time_t time_tt = std::mktime(&tm);

  // Convert time_t to system_clock::time_point
  t = std::chrono::system_clock::from_time_t(time_tt);

  // Handle subseconds (assuming microseconds for this example)
  // Adjust subsecond part based on its length (e.g., milliseconds or
  // microseconds)

  if (!subsecond_part.empty()) {
    const int max_digits = 9; // Assuming nanoseconds (9 digits)

    int subsecond_digits = subsecond_part.size();
    if (subsecond_digits > max_digits) {
      subsecond_part = subsecond_part.substr(0, max_digits);
    }
    int64_t subseconds = 0;
    try {
      subseconds = std::stoll(subsecond_part);
    } catch (...) {
      return false;
    }
    while (subsecond_digits < max_digits - 2) {
      subseconds *= 1000;
      subsecond_digits += 3;
    }
    while (subsecond_digits < max_digits) {
      subseconds *= 10;
      subsecond_digits++;
    }
    // Add subseconds as microseconds duration to the time_point
    t += std::chrono::nanoseconds(subseconds);
  }
  return true;
}

FileReader::FileReader() {}

TimeDataRow FileReader::decodeDataLine(const std::string &line) {
  auto values = split(line, ' ');
  TimeDataRow result = {};
  if (values.size() < 2) {
    return result; // todo: what should it return?
  }
  if (!parseTime(values[0] + " " + values[1], m_dateTimeFormat, result.t)) {
    return result;
  }

  for (size_t i = 2; i < values.size(); i++) {
    result.measures.push_back(std::stod(values[i]));
  }

  return result;
}

void FileReader::clear() {
  m_columns.clear();
  m_data.clear();
  m_readBytes = 0;
}

void FileReader::loadFile(const std::string &fileName) {
  std::ifstream fs(fileName); // Open the file
  if (!fs.is_open()) {        // Check if the file was opened successfully
    // std::cerr << "Failed to open file!" << std::endl;
    return;
  }
  //fs.size
  std::string line;
  if (std::getline(fs, line)) { // Read the first line
    if (m_columns.empty()) {
      m_columns = split(line, ' ');
    }
  }
  //todo: reaction to file size shrinking. fs.
  size_t i = 0;
  while (std::getline(fs, line)) { // Read the rest of the file
    if (i++ >= m_data.size()) {
      m_data.push_back(decodeDataLine(line));
    }
  }

  //m_readBytes = fs.postion;
  fs.close(); // Close the file when done
}
