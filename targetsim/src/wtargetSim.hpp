#pragma once

#include <QBrush>
#include <QFont>
#include <QPen>
#include <QWidget>
#include <memory>
#include <vector>

struct Target {
  double x, y;
  double r;
};

using TargetPtr = std::unique_ptr<Target>;
using Targets = std::vector<TargetPtr>;

class WTargetSim : public QWidget {
  Q_OBJECT
public:
  WTargetSim(double w, double h, QWidget *parent = nullptr);

  void addTarget(TargetPtr &&target);
  void removeTarget(size_t i);
  void clear();
  size_t count() { return m_targets.size(); }
  Target &targets(size_t i) { return *m_targets[i].get(); }

protected:
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
  void paintTargets(QPainter &painter);
  void paintTarget(QPainter &painter, double x, double y, double r);
  void paintTarget(QPainter &painter, double x, double y, double xs, double ys,
                   double r);

  double direction(double alpha, double beta, double alphaSpeed,
                   double betaSpeed);

  int tox(double alpha, double beta);
  int toy(double alpha, double beta);

  double toxf(double alpha, double beta);
  double toyf(double alpha, double beta);

  QBrush m_background;

  QPen m_targetPen;
  QBrush m_targetBrush;

  int w = 0;
  int h = 0;
  int m_margin = 2;
  double m_alphaWidth = qDegreesToRadians(70);
  double m_betaHeight = qDegreesToRadians(40);
  Targets m_targets;
};
