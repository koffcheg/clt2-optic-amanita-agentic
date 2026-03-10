#include "wChartAxes.hpp"
#include "utils.hpp"
#include "wChart.hpp"

#include <QPainter>
#include <cmath>
#include <fmt/format.h>
#include <iomanip>
#include <sstream>

double roundDown(double x) {
  if (x == 0) {
    return 0;
  }
  constexpr int x_count = 3;
  static const double xx[x_count] = {1, 2, 5};
  int s = x < 0 ? -1 : 1;
  x = abs(x);
  double p = floor(log10(x));
  x = x / pow(10, p);
  for (int i = x_count - 1; i >= 0; i--) {
    if (x > xx[i]) {
      return xx[i] * pow(10, p) * s;
    }
  }
  return xx[0] * pow(10, p) * s;
}

WChartAxis::WChartAxis(WChart &chart)
    : m_chart(chart), m_pen(Qt::black, 0), m_gridPen(Qt::gray, 0, Qt::DashLine),
      m_highlightPen(Qt::black, 0, Qt::SolidLine), m_font() {
  m_font.setPointSize(10);
  setMinMax(-1, 1);
}

WChartAxis::~WChartAxis(){};

void WChartAxis::setMinMax(double min, double max) {
  if (min < max) {
    m_minimum = min;
    m_maximum = max;
  } else {
    m_minimum = max;
    m_maximum = min;
  }
  m_minimum = u::coerce(m_minimum, m_minScale, m_maxScale);
  m_maximum = u::coerce(m_maximum, m_minScale, m_maxScale);
  m_chart.update();
}

double WChartAxis::toPixelf(double value) const {
  int s = startPos();
  int e = endPos();
  return s + (value - m_minimum) / (m_maximum - m_minimum) * (e - s);
}

double WChartAxis::toValue(int pixel) const {
  double s = startPos();
  double e = endPos();
  return m_minimum + (pixel - s) / (e - s) * (m_maximum - m_minimum);
}

void WChartAxis::setGrid(int grid) {
  m_grid = grid;
  m_chart.update();
}

void WChartAxis::setZero(bool value) {
  m_zero = value;
  m_chart.update();
}

void WChartAxis::setHighliteZero(bool value) {
  m_highlightZero = value;
  m_chart.update();
}

void WChartAxis::zoom(int pos, int delta) {
  if (!m_allowZoom) {
    return;
  }
  double s = delta < 0 ? m_scaleFactor : 1 / m_scaleFactor;
  double v = toValue(pos);
  double aMin = u::coerce(v - (v - m_minimum) * s, m_minScale, m_maxScale);
  double aMax = aMin + (m_maximum - m_minimum) * s;
  setMinMax(aMin, aMax);
}

void WChartAxis::pan(int delta) {
  if (!m_allowPan) {
    return;
  }
  double d = m_maximum - m_minimum;
  double aMin = toValue(toPixelf(m_minimum) + delta);
  double aMax = aMin + d;
  setMinMax(aMin, aMax);
}

/// WChartXAxis

WChartXAxis::WChartXAxis(WChart &chart) : WChartAxis(chart), m_top(false) {}

int WChartXAxis::startPos() const { return m_chart.marginLeft(); }

int WChartXAxis::endPos() const {
  return m_chart.width() - m_chart.marginRight();
}

void WChartXAxis::paint(QPainter &painter, QPaintEvent * /*event*/) {
  painter.save();
  painter.setPen(m_pen);
  painter.setFont(m_font);

  int x1 = m_chart.marginLeft();
  int x2 = m_chart.width() - m_chart.marginRight();

  int y;
  if (m_zero && m_maximum * m_minimum <= 0) {
    y = m_chart.yAxis().toPixel(0);
  } else {
    y = m_top ? m_chart.marginTop() : m_chart.height() - m_chart.marginBottom();
  }
  painter.drawLine(x1, y, x2, y);

  QFontMetrics fm(m_font);
  int precision = ceil(-log10((m_maximum - m_minimum)));
  precision = (precision > 1) ? precision : 1;
  std::string ss = fmt::format("{:.{}f}", m_minimum, precision);
  QString s = QString::fromStdString(ss);
  int tickCount = m_chart.width() / (fm.horizontalAdvance(s) * 3);
  double increment = roundDown((m_maximum - m_minimum) / tickCount);

  if (increment != 0) {
    int yg1 = m_chart.plotRect().top();
    int yg2 = m_chart.plotRect().bottom();

    int y1 = y - 2;
    int y2 = y + 2;
    int i = 0;
    double t0 = floor(m_minimum / increment) * increment;
    double t = t0;
    int precision = ceil(-log10(increment));
    precision = (precision > 0) ? precision : 0;

    if (m_highlightZero) {
      int x = toPixel(0);
      if (x >= m_chart.marginLeft() && x <= m_chart.plotRect().right()) {
        painter.setPen(m_highlightPen);
        painter.drawLine(x, yg1, x, yg2);
      }
    }

    while (t <= m_maximum) {
      int x = toPixel(t);
      if (x >= m_chart.marginLeft() && x <= m_chart.plotRect().right()) {
        painter.setPen(m_gridPen);
        painter.drawLine(x, yg1, x, yg2);
        painter.setPen(m_pen);
        painter.drawLine(x, y1, x, y2);
        std::string ss = fmt::format("{:.{}f}", t, precision);
        QString s = QString::fromStdString(ss);
        x -= fm.horizontalAdvance(s) / 2;
        if (m_top) {
          painter.drawText(x, y1 - 2, s);
        } else {
          painter.drawText(x, y2 + fm.height(), s);
        }
      }
      i++;
      t = t0 + increment * i;
    };
  }
  painter.restore();
}

void WChartXAxis::mousePressEvent(QMouseEvent * /*event*/) {}

void WChartXAxis::setTop(bool value) {
  m_top = value;
  m_chart.update();
}

WChartYAxis::WChartYAxis(WChart &chart) : WChartAxis(chart), m_right(false) {}

void WChartYAxis::paint(QPainter &painter, QPaintEvent * /*event*/) {
  painter.save();
  painter.setPen(m_pen);
  painter.setFont(m_font);

  int x;
  if (m_zero && m_maximum * m_minimum <= 0) {
    x = m_chart.xAxis().toPixel(0);
  } else {
    x = m_right ? m_chart.width() - m_chart.marginRight()
                : m_chart.marginLeft();
  }
  int y1 = m_chart.marginTop();
  int y2 = m_chart.height() - m_chart.marginBottom();
  painter.drawLine(x, y1, x, y2);

  QFontMetrics fm(m_font);
  int tickCount = m_chart.height() / (fm.height() * 3);
  double increment = roundDown((m_maximum - m_minimum) / tickCount);

  if (increment != 0) {
    int xg1 = m_chart.marginLeft();
    int xg2 = m_chart.width() - m_chart.marginRight();
    int x1 = x - 2;
    int x2 = x + 2;
    int i = 0;
    double t0 = floor(m_minimum / increment) * increment;
    double t = t0;
    int precision = ceil(-log10(increment));
    precision = (precision > 0) ? precision : 0;

    if (m_highlightZero) {
      int y = toPixel(0);
      if (y >= m_chart.plotRect().top() && y <= m_chart.plotRect().bottom()) {
        painter.setPen(m_highlightPen);
        painter.drawLine(xg1, y, xg2, y);
      }
    }

    while (t <= m_maximum) {
      int y = toPixel(t);
      if (y >= m_chart.plotRect().top() && y <= m_chart.plotRect().bottom()) {
        painter.setPen(m_gridPen);
        painter.drawLine(xg1, y, xg2, y);
        painter.setPen(m_pen);
        painter.drawLine(x1, y, x2, y);
        std::string ss = fmt::format("{:.{}f}", t, precision);
        QString s = QString::fromStdString(ss);
        if (m_right) {
          painter.drawText(x2 + 2, y + (fm.height() * 1) / 4, s);
        } else {
          painter.drawText(x1 - fm.horizontalAdvance(s) - 2, y, s);
        }
      }
      i++;
      t = t0 + increment * i;
    }
  }
  painter.restore();
}
int WChartYAxis::startPos() const {
  return m_chart.height() - m_chart.marginBottom();
}

int WChartYAxis::endPos() const { return m_chart.marginTop(); }

void WChartYAxis::mousePressEvent(QMouseEvent * /*event*/) {}

void WChartYAxis::setRight(bool value) {
  m_right = value;
  m_chart.update();
}
