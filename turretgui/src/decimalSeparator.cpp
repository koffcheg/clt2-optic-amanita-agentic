#include "decimalSeparator.hpp"
#include <clocale>
#include <iomanip>
#include <iostream>
#include <locale>

struct DecimalSeparator : std::numpunct<char> {
  DecimalSeparator(char separator)
      : std::numpunct<char>(), m_decimalSeparator(separator) {}

protected:
  char do_decimal_point() const override { return m_decimalSeparator; }

private:
  char m_decimalSeparator;
};

// Setting a locale with a custom decimal separator (point)
void setDecimalSeparator(char separator) {
  // Force '.' as the radix point.
  std::setlocale(LC_NUMERIC, "C");

  // Create a locale with point as the decimal separator
  std::locale customLocale(std::locale(), new DecimalSeparator(separator));
  std::cout.imbue(customLocale);
}
