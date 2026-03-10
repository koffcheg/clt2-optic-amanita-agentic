#pragma once

#include <QBrush>
#include <QFont>
#include <QPen>
#include <QWidget>
#include <list>

struct SkyPosition {
  double alpha;
  double beta;
  // int time; //???
};

struct SkyTarget {
  unsigned long id;
  SkyPosition position;
  SkyPosition speed;

  bool selected;
  std::string label;
  QColor color;
};

using SkyTargets = std::list<SkyTarget>;

struct SkyTrajectory {
  std::list<SkyPosition> path;
  bool selected;
  bool visible;
  std::string label;
  QColor color;
};

// using SkyTrajectories = std::list<SkyTrajectory>;
using SkyTrajectories = std::map<int, SkyTrajectory>;

class WSkyChart : public QWidget {
  Q_OBJECT
public:
  WSkyChart(QWidget *parent);

  void setPos(double alpha, double beta);

  SkyTargets &targets() { return m_targets; };
  SkyTrajectories &trajectories() { return m_trajectories; };
  SkyTarget *selectedTarget();

  void selectTarget(SkyTarget *target, QMouseEvent *event);   


signals:
    void onTargetSelected(const SkyTarget* target);

public slots:
  void animate();

protected:
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
private:
  void paintGrid(QPainter &painter);
  void paintTurretPos(QPainter &painter);
  void paintTargets(QPainter &painter);
  void paintTrajectories(QPainter &painter);
  void paintLabels(QPainter &painter);
  void paintTarget(QPainter &painter, double alpha, double beta,
                   double alphaSpeed, double betaSpeed);

  double direction(double alpha, double beta, double alphaSpeed,
                   double betaSpeed);

  int tox(double alpha, double beta);
  int toy(double alpha, double beta);

  double toxf(double alpha, double beta);
  double toyf(double alpha, double beta);

  QBrush m_background;
  QPen m_gridPen;
  QPen m_grid2Pen;
  QFont m_gridFont;

  QPen m_turretPen;
  QBrush m_turretBrush;
  QFont m_turretFont;

  QPen m_targetPen;
  QBrush m_targetBrush;
  QFont m_targetFont;

  QPen m_trajectoryPen;

  bool m_left;
  double m_alpha0;

  int w = 0;
  int h = 0;
  int r = 0;
  int m_center = 0;
  int m_margin = 2;

  double m_alpha = 0;
  double m_beta = 0;
  SkyTargets m_targets;
  SkyTrajectories m_trajectories;
};
