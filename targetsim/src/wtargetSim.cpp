#include "wtargetSim.hpp"
#include "utils.hpp"

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QWidget>
#include <algorithm>
#include <string>

WTargetSim::WTargetSim(double w, double h, QWidget *parent)
    : QWidget(parent), m_background(Qt::black), m_targetPen(Qt::white),
      m_targetBrush(Qt::white), m_alphaWidth(w), m_betaHeight(h) {
  // setFixedSize(400, 400);
  // setBaseSize(200, 200);
  // todo: resize(400, 400);
}

void WTargetSim::paintEvent(QPaintEvent *event) {
  QPainter painter;
  painter.begin(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // todo: optimize redraw. Save background to the bitmap?
  painter.fillRect(event->rect(), m_background);
  // painter.translate(m_center, m_center);

  paintTargets(painter);

  painter.end();
}

void WTargetSim::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
  w = event->size().width();
  h = event->size().height();
  // r = std::min(w, h) / 2 - 30;
  // m_center = std::min(w, h) / 2;
}

void WTargetSim::mousePressEvent(QMouseEvent * /*event*/) {
  /*int xm = event->position().x() - m_center;
  int ym = event->position().y() - m_center;
  int mind = 9999999; // todo:
  update();*/
}

void WTargetSim::mouseDoubleClickEvent(QMouseEvent * /*event*/) {
  if (isFullScreen()) {
    showNormal();
  } else {
    showFullScreen();
  }
}

double WTargetSim::toxf(double alpha, double /*beta*/) {
  return w / 2 + w * alpha / m_alphaWidth;
}

double WTargetSim::toyf(double /*alpha*/, double beta) {
  return h / 2 - h * beta / m_betaHeight;
}

int WTargetSim::tox(double alpha, double beta) {
  return round(toxf(alpha, beta));
}

int WTargetSim::toy(double alpha, double beta) {
  return round(toyf(alpha, beta));
}

double WTargetSim::direction(double alpha, double beta, double alphaSpeed,
                             double betaSpeed) {
  if (alphaSpeed == 0 && betaSpeed == 0) {
    return NAN;
  }
  double x0, y0;
  double x1, y1;
  double dt = 0.05;
  do {
    x0 = toxf(alpha - alphaSpeed, beta - betaSpeed);
    y0 = toyf(alpha - alphaSpeed, beta - betaSpeed);
    x1 = toxf(alpha + alphaSpeed, beta + betaSpeed);
    y1 = toyf(alpha + alphaSpeed, beta + betaSpeed);

    if (dt > 10) { // limit
      break;
    }
    dt *= 2;
  } while ((abs(x1 - x0) < 1) && (abs(y1 - y0) < 1));
  return atan2(-(y1 - y0), x1 - x0);
}

void WTargetSim::paintTarget(QPainter &painter, double x, double y, double r) {
  painter.drawEllipse(tox(x, y) - r, toy(x, y) - r, 2 * r, 2 * r);
}

void WTargetSim::paintTarget(QPainter &painter, double x, double y, double xs,
                             double ys, double r) {
  double x1 = 3 * r;
  double x2 = r;
  double y1 = r;
  x = toxf(x, y);
  y = toyf(x, y);
  double d = direction(x, y, xs, ys);

  if (isnan(d)) {
    constexpr double pr = 2;
    painter.drawEllipse(x - pr, y - pr, 2 * pr, 2 * pr);
    return;
  }

  double c = cos(d);
  double s = sin(d);

  const QPointF points[4] = {
      QPointF(x + (+x1) * c, y - x1 * s - (0) * c),
      QPointF(x + (-x2) * c - y1 * s, y + x2 * s - (+y1) * c),
      QPointF(x + (-x2) * c + y1 * s, y + x2 * s - (-y1) * c),
      QPointF(x + (+x1) * c, y - (+x1) * s - (0) * c),
  };

  painter.drawPolygon(points, 4);
}

void WTargetSim::paintTargets(QPainter &painter) {
  QPen pen = m_targetPen;
  painter.save();
  painter.setPen(pen);
  painter.setBrush(m_targetBrush);
  for (auto &t : m_targets) {
    pen.setWidth(1);
    painter.setPen(pen);
    paintTarget(painter, t->x, t->y, t->r);
  }
  painter.restore();
}

void WTargetSim::addTarget(TargetPtr &&target) {
  m_targets.push_back(std::move(target));
  update();
}

void WTargetSim::removeTarget(size_t i) {
  m_targets.erase(m_targets.begin() + i);
  update();
}

void WTargetSim::clear() {
  m_targets.clear();
  update();
}
