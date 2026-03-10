#include "wChartLegend.hpp"
#include "wChart.hpp"
#include "wChartAxes.hpp"

#include <QMouseEvent>
#include <QPainter>
#include <QWidget>
#include <cmath>

WChartLegend::WChartLegend(WChart &chart)
    : m_chart(chart), m_pen(Qt::lightGray, 0), m_brush(Qt::white), m_font() {
  m_font.setPointSize(12);
}

WChartLegend::~WChartLegend(){};

void WChartLegend::setPen(QPen &pen) {
  m_pen = pen;
  m_chart.update();
};

void WChartLegend::paint(QPainter &painter) {
  if (!m_visible || !m_chart.plots().size()) {
    return;
  }
  painter.save();
  painter.setPen(m_pen);
  painter.setBrush(m_brush);

  QFontMetrics fm(m_font);
  int vspace = 3;
  int hspace = 5;
  int lineWidth = 10;
  m_vpitch = fm.height() + vspace;

  int h = m_chart.plots().size() * m_vpitch + 2 * vspace;
  int w = 0;

  for (size_t i = 0; i < m_chart.plots().size(); i++) {
    w = fmax(w, fm.horizontalAdvance(m_chart.plots(i).title()));
  }
  w += hspace * 3 + lineWidth;

  m_width = w;
  m_height = h;

  painter.drawRect(m_left, m_top, w, h);
  painter.setClipRect(m_left, m_top, w, h);

  int x1 = m_left + hspace;
  int x2 = x1 + lineWidth;
  int x3 = x2 + hspace;

  for (size_t i = 0; i < m_chart.plots().size(); i++) {
    int y = m_top + i * m_vpitch + fm.height();
    int y1 = y - fm.xHeight();
    if (m_chart.plots(i).visible()) {
      painter.setPen(m_chart.plots(i).pen());
    } else {
      painter.setPen(m_pen);
    }
    painter.drawLine(x1, y1, x2, y1);
    painter.drawText(x3, y, m_chart.plots(i).title());
  }

  painter.restore();
}

void WChartLegend::setVisible(bool visible) {
  m_visible = visible;
  m_chart.update();
};

void WChartLegend::mousePressEvent(QMouseEvent *event) {
  int x = event->position().x();
  int y = event->position().y();
  if (x < m_left || x > m_left + m_width) {
    return;
  }
  if (y < m_top || y > m_top + m_height) {
    return;
  }
  int idx = (y - m_top) / m_vpitch;
  if (idx >= 0 && (size_t)idx < m_chart.plots().size()) {
    m_chart.plots(idx).setVisible(!m_chart.plots(idx).visible());
  }
}
