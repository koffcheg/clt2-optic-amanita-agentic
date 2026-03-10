#pragma once

#include <QFont>
#include <QPen>
#include <QWidget>

class WChart;

class WChartAxis {
public:
  WChartAxis(WChart &chart);
  virtual ~WChartAxis();

  virtual void paint(QPainter & /*painter*/, QPaintEvent * /*event*/) {};
  virtual void resize(QResizeEvent * /*event*/) {};
  virtual void mousePressEvent(QMouseEvent * /*event*/) {};

  double minimum() { return m_minimum; }
  double maximum() { return m_maximum; }

  void setMinMax(double min, double max);

  double toPixelf(double value) const;
  int toPixel(double value) const { return round(toPixelf(value)); };
  double toValue(int pixel) const;

  virtual int startPos() const = 0;
  virtual int endPos() const = 0;

  bool zero() { return m_zero; }
  void setZero(bool value);

  bool highlightZero() { return m_highlightZero; }
  void setHighliteZero(bool value);

  int grid() const { return m_grid; }
  void setGrid(int grid);
  void zoom(int pos, int delta);
  void pan(int delta);

protected:
  WChart &m_chart;
  QPen m_pen;
  QPen m_gridPen;
  QPen m_highlightPen;
  QFont m_font;
  double m_minimum;
  double m_maximum;
  double m_minScale = -INFINITY;
  double m_maxScale =  INFINITY;
  double m_scaleFactor = 1.25;
  int m_grid = 2;
  bool m_zero = false;
  bool m_highlightZero = false;
  bool m_allowZoom = true;
  bool m_allowPan = true;
};

class WChartXAxis : public WChartAxis {
public:
  WChartXAxis(WChart &chart);
  // virtual ~WChartAxis();

  void paint(QPainter &painter, QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent * /*event*/) override;

  bool top() { return m_top; }
  void setTop(bool value);

  int startPos() const override;
  int endPos() const override;

protected:
private:
  bool m_top;
};

class WChartYAxis : public WChartAxis {
public:
  WChartYAxis(WChart &chart);
  //~WChartAxis() override;

  void paint(QPainter &painter, QPaintEvent * /*event*/) override;
  void mousePressEvent(QMouseEvent * /*event*/) override;

  bool right() { return m_right; }
  void setRight(bool value);

  int startPos() const override;
  int endPos() const override;

protected:
private:
  bool m_right;
};
