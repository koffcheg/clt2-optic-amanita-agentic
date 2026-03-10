#include "wSkyChart.hpp"
#include "utils.hpp"

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QTimer>
#include <QWidget>
#include <algorithm>
#include <string>

WSkyChart::WSkyChart(QWidget *parent)
    : QWidget(parent), m_background(Qt::white), m_gridPen(Qt::darkGray),
      m_grid2Pen(Qt::gray), m_gridFont(), m_turretPen(Qt::darkGreen),
      m_turretBrush(Qt::NoBrush), m_turretFont(), m_targetPen(Qt::red),
      m_targetBrush(Qt::NoBrush), m_targetFont(), m_trajectoryPen(Qt::darkGray),
      m_left(true), m_alpha0(0) {
  m_gridFont.setPointSize(10);
  m_turretFont.setPointSize(14);
  // setFixedSize(400, 400);
  // setBaseSize(200, 200);
  // todo: resize(400, 400);
}

void WSkyChart::setPos(double alpha, double beta) {
  m_alpha = alpha;
  m_beta = beta;
  update();
}

void WSkyChart::animate() {
  // elapsed = (elapsed + qobject_cast<QTimer*>(sender())->interval()) % 1000;
  update();
}

void WSkyChart::paintEvent(QPaintEvent *event) {
  QPainter painter;
  painter.begin(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // todo: optimize redraw. Save background to the bitmap?
  painter.fillRect(event->rect(), m_background);
  painter.translate(m_center, m_center);

  paintGrid(painter);
  paintTurretPos(painter);
  paintLabels(painter);
  paintTrajectories(painter);
  paintTargets(painter);

  painter.end();
}

void WSkyChart::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
  w = event->size().width();
  h = event->size().height();
  r = std::min(w, h) / 2 - 30;
  m_center = std::min(w, h) / 2;
}

void WSkyChart::mousePressEvent(QMouseEvent *event) {
  int xm = event->position().x() - m_center;
  int ym = event->position().y() - m_center;
  int mind = 9999999; // todo:
  SkyTarget *target = nullptr;

  for (auto &t : m_targets) {
    int x = tox(t.position.alpha, t.position.beta);
    int y = toy(t.position.alpha, t.position.beta);
    int d = (x - xm) * (x - xm) + (y - ym) * (y - ym);

    if (d < 25 && d < mind) {
      target = &t;
      mind = d;
    }
  }
  selectTarget(target, event);
  update();
}

double WSkyChart::toxf(double alpha, double beta) {
  if (m_left) {
    alpha = -alpha;
  }
  alpha += M_PI_2 + m_alpha0;

  return cos(alpha) * r * (1 - beta / M_PI_2);
}

double WSkyChart::toyf(double alpha, double beta) {
  if (m_left) {
    alpha = -alpha;
  }
  alpha += M_PI_2 + m_alpha0;
  return -sin(alpha) * r * (1 - beta / M_PI_2);
}

int WSkyChart::tox(double alpha, double beta) {
  return round(toxf(alpha, beta));
}

int WSkyChart::toy(double alpha, double beta) {
  return round(toyf(alpha, beta));
}

double WSkyChart::direction(double alpha, double beta, double alphaSpeed,
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

void WSkyChart::paintGrid(QPainter &painter) {
  painter.save();
  painter.setPen(m_gridPen);
  painter.setFont(m_gridFont);
  painter.drawLine(-r, 0, r, 0);
  painter.drawLine(0, -r, 0, r);

  int increment = r < 200 ? 15 : 10;

  for (int i = 0; i < 90; i += increment) {
    int rr = r * (90 - i) / 90;
    painter.drawEllipse(-rr, -rr, 2 * rr, 2 * rr);

    painter.drawText(-rr, 0, 20, 20, 0,
                     QString::fromStdString(std::to_string(i)));
  }

  increment = r < 100 ? 45 : r < 200 ? 30 : 15;
  for (int i = 0; i < 360; i += increment) {
    int x = tox(i * M_PI / 180.0, 0);
    int y = toy(i * M_PI / 180.0, 0);
    painter.drawLine(0, 0, x, y);
    if (y < 0) {
      y -= 20;
    };
    if (y == 0) {
      y -= 10;
    };
    if (x < 0) {
      x -= 30;
    };
    std::string s;
    s = std::to_string(i <= 180 ? i : i - 360);
    painter.drawText(x, y, 30, 20, 0, QString::fromStdString(s));
  }
  painter.restore();
}

void WSkyChart::paintTurretPos(QPainter &painter) {
  int pr = 5;
  int pi = 1;
  int x = tox(m_alpha, m_beta);
  int y = toy(m_alpha, m_beta);

  painter.save();
  painter.setPen(m_turretPen);
  painter.setBrush(m_turretBrush);
  painter.setFont(m_turretFont);

  painter.drawEllipse(x - pr, y - pr, 2 * pr, 2 * pr);
  painter.drawEllipse(x - pi, y - pi, 2 * pi, 2 * pi);
  // painter->drawPoint(x, y);

  QFontMetrics fm(m_turretFont);
  static char fmt_buff[64];

  std::string alpha = "𝛼:";
  if (!isnan(m_alpha)) {
    sprintf(fmt_buff, "%.2lf", u::rad2deg(m_alpha));
    alpha += fmt_buff;
  } else
    alpha += "-.--°";
  painter.drawText(-m_center + m_margin, -m_center + 1 * fm.height(),
                   QString::fromStdString(alpha));
  std::string beta = "𝛽:";
  if (!isnan(m_beta)) {
    sprintf(fmt_buff, "%.2lf", u::rad2deg(m_beta));
    beta += fmt_buff;
  } else
    beta += "-.--°";
  painter.drawText(-m_center + m_margin, -m_center + 2 * fm.height(),
                   QString::fromStdString(beta));

  painter.restore();
}

void WSkyChart::paintLabels(QPainter &painter) {
  painter.save();
  painter.restore();
}

void WSkyChart::paintTarget(QPainter &painter, double alpha, double beta,
                            double alphaSpeed, double betaSpeed) {
  constexpr double x1 = 6;
  constexpr double x2 = 3;
  constexpr double y1 = 2;
  double x = toxf(alpha, beta);
  double y = toyf(alpha, beta);
  double d = direction(alpha, beta, alphaSpeed, betaSpeed);

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

void WSkyChart::paintTargets(QPainter &painter) {
  QPen pen = m_targetPen;
  SkyTarget *selectedTarget = nullptr;
  painter.save();
  painter.setPen(pen);
  painter.setBrush(m_targetBrush);
  for (auto &t : m_targets) {
    pen.setWidth(t.selected ? 2 : 1);
    if (t.selected) {
      selectedTarget = &t;
    }
    painter.setPen(pen);
    paintTarget(painter, t.position.alpha, t.position.beta, t.speed.alpha,
                t.speed.beta);
  }

  if (selectedTarget) {
    static char fmt_buff[64];
    QFontMetrics fm(m_turretFont);
    std::string alpha = "𝛼:";
    if (!isnan(m_alpha)) {
      sprintf(fmt_buff, "%.2lf", u::rad2deg(selectedTarget->position.alpha));
      alpha += fmt_buff;
    } else
      alpha += "-.--°";
    painter.drawText(m_center / 2, -m_center + 1 * fm.height(),
                     QString::fromStdString(alpha));
    std::string beta = "𝛽:";
    if (!isnan(m_beta)) {
      sprintf(fmt_buff, "%.2lf", u::rad2deg(selectedTarget->position.beta));
      beta += fmt_buff;
    } else
      beta += "-.--°";
    painter.drawText(m_center / 2, -m_center + 2 * fm.height(),
                     QString::fromStdString(beta));
  }

  painter.restore();
}

void WSkyChart::paintTrajectories(QPainter &painter) {
  QPen pen = m_trajectoryPen;

  painter.save();

  for (auto &trajectory : m_trajectories) {
    int x1, x2 = 0;
    int y1, y2 = 0;
    bool first = true;
    pen.setColor(trajectory.second.color);
    painter.setPen(pen);
    for (auto &p : trajectory.second.path) {
      x1 = x2;
      y1 = y2;
      x2 = tox(p.alpha, p.beta);
      y2 = toy(p.alpha, p.beta);
      if (!first) {
        painter.drawLine(x1, y1, x2, y2);
      }
      first = false;
    }
  }

  painter.restore();
}
void WSkyChart::selectTarget(SkyTarget *target, QMouseEvent * /*event*/) {
  for (auto &t : m_targets) {
    t.selected = false;
  }
  if (target) {
    target->selected = true;
  }
  emit onTargetSelected(target);
}

SkyTarget *WSkyChart::selectedTarget() {
  for (auto &t : m_targets) {
    if (t.selected) {
      return &t;
    }
  }
  return nullptr;
}
