#include "wLed.hpp"

#include <QPaintEvent>
#include <QPainter>

WLed::WLed(QWidget *parent)
    : QWidget(parent), m_brush(Qt::darkGray), m_pen(Qt::black, 2) {
  setFixedSize(12, 12);
}

void WLed::paintEvent(QPaintEvent * /*event*/) {
  QPainter painter;
  int w = m_pen.width();
  int w1 = m_pen.width() + 1;
  painter.begin(this);
  painter.setBrush(m_brush);
  painter.setPen(m_pen);
  painter.drawEllipse(w, w, width() - w1, height() - w1);
  painter.end();
}

void WLed::setColor(QColor color) {
  if (m_brush.color() != color) {
    m_brush.setColor(color);
    update();
  }
}

void WLed::setSize(int size) { setFixedSize(size, size); }
