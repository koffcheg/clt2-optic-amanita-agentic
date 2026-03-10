#pragma once

#include <QBrush>
#include <QPen>
#include <QWidget>

class WLed : public QWidget {
  Q_OBJECT

public:
  WLed(QWidget *parent);

  void setColor(QColor color);
  void setSize(int size);

  QColor color() { return m_brush.color(); }
  int size() { return width(); };

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  QBrush m_brush;
  QPen m_pen;
};
