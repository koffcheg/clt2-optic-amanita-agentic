#pragma once

#include <QFont>
#include <QMouseEvent>
#include <QPen>
#include <QWidget>
#include <memory>
#include <vector>

class WChart;

class WChartLegend {
public:
  WChartLegend(WChart &chart);
  virtual ~WChartLegend();

  virtual void paint(QPainter &painter);

  QPen &pen() { return m_pen; }
  void setPen(QPen &pen);
  void setVisible(bool visible);
  bool visible() { return m_visible; }

  void mousePressEvent(QMouseEvent *event);

protected:
  WChart &m_chart;
  QPen m_pen;
  QBrush m_brush;
  QFont m_font;
  bool m_visible = true;

  // todo: adjust position
  int m_top = 25;
  int m_left = 40;

  int m_width = 0;
  int m_height = 0;
  int m_vpitch = 0;
};
