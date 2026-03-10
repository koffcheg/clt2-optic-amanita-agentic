#include "wChartPlot.hpp"
#include "wChart.hpp"
#include "wChartAxes.hpp"

#include <QPainter>
#include <cmath>
//#include <format>

WChartPlot::WChartPlot(WChart &chart, const QString &title)
    : m_chart(chart), m_pen(Qt::red, 0), m_font(), m_title(title) {
  m_font.setPointSize(10);
}

WChartPlot::~WChartPlot(){};

void WChartPlot::setPen(QPen &pen) {
  m_pen = pen;
  m_chart.update();
};

void WChartPlot::append(ChartPoint &point) { append(point.x, point.y); }
void WChartPlot::append(double x, double y) {
  m_points.push_back(ChartPoint{x, y});

  if (x > m_maximiumX) {
    m_maximiumX = x;
  }
  if (x < m_minimumX) {
    m_minimumX = x;
  }
  if (y > m_maximumY) {
    m_maximumY = y;
  }
  if (y < m_minimumY) {
    m_minimumY = y;
  }

  m_chart.update();
}

void WChartPlot::clear() {
  m_points.clear();
  m_minimumX = INFINITY;
  m_maximiumX = -INFINITY;
  m_minimumY = INFINITY;
  m_maximumY = -INFINITY;
  m_chart.update();
}

void WChartPlot::paint(QPainter &painter) {
  if (!m_visible) {
    return;
  }
  if (m_points.size() < 2) {
    return;
  }
  painter.save();
  painter.setPen(m_pen);
  painter.setClipRect(m_chart.plotRect());

  int x2 = m_chart.xAxis().toPixel(m_points.at(0).x);
  int y2 = m_chart.yAxis().toPixel(m_points.at(0).y);

  for (size_t i = 1; i < m_points.size(); i++) {
    int x1 = x2;
    int y1 = y2;
    x2 = m_chart.xAxis().toPixel(m_points.at(i).x);
    y2 = m_chart.yAxis().toPixel(m_points.at(i).y);

    painter.drawLine(x1, y1, x2, y2);
  }

  painter.restore();
}

void WChartPlot::setVisible(bool visible) {
  m_visible = visible;
  m_chart.update();
}
