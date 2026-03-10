#pragma once

#include "wChartAxes.hpp"
#include "wChartLegend.hpp"
#include "wChartPlot.hpp"
#include <QMouseEvent>
#include <QWidget>

class SkyChart;

class WChart : public QWidget {
  Q_OBJECT

public:
  WChart(QWidget *parent);

  const QRect &plotRect() { return m_plotRect; }
  int marginLeft() { return m_marginLeft; };
  int marginRight() { return m_marginRight; };
  int marginTop() { return m_marginTop; };
  int marginBottom() { return m_marginBottom; };

  void setMargins(int left, int right, int top, int bottom);

  WChartXAxis &xAxis() { return m_xAxis; };
  WChartYAxis &yAxis() { return m_yAxis; };

  WChartPlots &plots() { return m_plots; }
  WChartPlot &plots(int idx) { return *m_plots[idx].get(); }
  WChartPlot &operator[](int idx) { return *m_plots[idx].get(); }

  void addPlot(WChartPlotPtr plot);
  void removePlot(const WChartPlotPtr &plot);
  void removePlot(int index);
  void clearPlots();

protected:
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

  WChartXAxis m_xAxis;
  WChartYAxis m_yAxis;
  WChartLegend m_legend;

  WChartPlots m_plots;

private:
  QBrush m_background;

  QRect m_plotRect;
  int m_marginLeft;
  int m_marginRight;
  int m_marginTop;
  int m_marginBottom;
  bool m_panMode = false;
  QPointF m_panStartPos;
};
