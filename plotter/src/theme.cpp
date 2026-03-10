
#include "theme.hpp"
#include "utils.hpp"

Theme theme;

const QColor &Theme::plotColor(int i) {
  static QColor colors[] = {Qt::red,         Qt::blue,       Qt::green,
                        Qt::darkRed,     Qt::darkBlue,   Qt::darkGreen,
                        Qt::magenta,     Qt::darkYellow, Qt::darkCyan,
                        Qt::darkMagenta, Qt::black,      Qt::darkGray,
                        Qt::cyan, Qt::yellow};

  return colors[i % u::countof(colors)];
}
