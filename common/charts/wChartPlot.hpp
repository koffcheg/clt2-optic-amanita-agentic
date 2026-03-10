#pragma once

#include <QFont>
#include <QPen>
#include <QWidget>
#include <memory>
#include <vector>

class WChart;

struct ChartPoint {
  double x;
  double y;
};

using ChartPoints = std::vector<ChartPoint>;

class WChartPlot {
public:
  WChartPlot(WChart &chart, const QString& title="");
  virtual ~WChartPlot();

  virtual void paint(QPainter &painter);

  QPen &pen() { return m_pen; }
  void setPen(QPen &pen);

  const ChartPoint &operator[](int idx) { return m_points[idx]; }
  void append(ChartPoint &point);
  void append(double x, double y);
  size_t size() { return m_points.size(); }
  void clear();
  const QString &title() { return m_title; }

  bool visible() {return m_visible;}
  void setVisible(bool);

protected:
  WChart &m_chart;
  QPen m_pen;
  QFont m_font;
  double m_minimumX = INFINITY;
  double m_maximiumX = -INFINITY;
  double m_minimumY = INFINITY;
  double m_maximumY = -INFINITY;

  ChartPoints m_points;
  // double m_increment;
  bool m_visible = true;
  QString m_title;
};

using WChartPlotPtr = std::unique_ptr<WChartPlot>;

using WChartPlots = std::vector<WChartPlotPtr>;
