#include "wChart.hpp"

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>

WChart::WChart(QWidget *parent)
    : QWidget(parent), m_xAxis(*this), m_yAxis(*this), m_legend(*this),
      m_background(Qt::white), m_marginLeft(30), m_marginRight(50),
      m_marginTop(15), m_marginBottom(25) {
  // setFixedSize(400, 400);
  // setBaseSize(200, 200);
  resize(400, 400);
}

void WChart::paintEvent(QPaintEvent *event) {
  QPainter painter;
  painter.begin(this);
  painter.setRenderHint(QPainter::Antialiasing);
  // m_skyChart.paint(&painter, event);

  painter.fillRect(event->rect(), m_background);

  // size()
  m_xAxis.paint(painter, event);
  m_yAxis.paint(painter, event);
  for (auto &plot : m_plots) {
    plot->paint(painter);
  }
  m_legend.paint(painter);
  painter.end();
}

void WChart::wheelEvent(QWheelEvent *event) {
  if (event->modifiers() & Qt::ControlModifier) {
    xAxis().zoom(event->position().x(), event->angleDelta().y());
  } else {
    yAxis().zoom(event->position().y(), event->angleDelta().y());
  }
  event->accept();
}

void WChart::mousePressEvent(QMouseEvent *event) {
  QWidget::mousePressEvent(event);
  if (event->button() & Qt::RightButton) {
    m_panMode = true;
    m_panStartPos = event->position();
  }
  m_legend.mousePressEvent(event);
  update();
}

void WChart::mouseMoveEvent(QMouseEvent *event) {
  QWidget::mouseMoveEvent(event);
  if (m_panMode) {
    m_xAxis.pan(m_panStartPos.x() - event->position().x());
    m_yAxis.pan(m_panStartPos.y() - event->position().y());
    m_panStartPos = event->position();
  }
}

void WChart::mouseReleaseEvent(QMouseEvent *event) {
  QWidget::mouseReleaseEvent(event);
  m_panMode = false;
}

void WChart::setMargins(int left, int right, int top, int bottom) {
  m_marginLeft = left;
  m_marginRight = right;
  m_marginTop = top;
  m_marginBottom = bottom;
  m_plotRect.setCoords(m_marginLeft, m_marginTop, width() - m_marginRight,
                       height() - m_marginBottom);
}

void WChart::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
  m_xAxis.resize(event);
  m_yAxis.resize(event);
  m_plotRect.setCoords(m_marginLeft, m_marginTop, width() - m_marginRight,
                       height() - m_marginBottom);
}

void WChart::addPlot(WChartPlotPtr plot) {
  m_plots.push_back(std::move(plot));
  // todo: m_chart.update();
}

void WChart::removePlot(const WChartPlotPtr &plot) {
  auto it =
      std::find_if(m_plots.begin(), m_plots.end(),
                   [&plot](const WChartPlotPtr &ptr) { return ptr == plot; });

  // Check if the element was found and erase it
  if (it != m_plots.end()) {
    m_plots.erase(it);
  }
  // todo: m_chart.update();
}

void WChart::removePlot(int index) {
  m_plots.erase(m_plots.begin() + index);
  // todo: m_chart.update();
}

void WChart::clearPlots() { m_plots.clear(); }
