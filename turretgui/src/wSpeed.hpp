#pragma once

#include <QFont>
#include <QLabel>
#include <QLineEdit>
#include <QPen>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <memory>
#include <vector>
#include "application.hpp"

class WSpeed : public QWidget {
  Q_OBJECT
public:
  WSpeed(QWidget *parent, turret::Application &application);
  virtual ~WSpeed();

  // virtual void paint(QPainter & /*painter*/, QPaintEvent * /*event*/);
  void onGotoClicked();
  void onStopClicked();

protected:
private:
  turret::Application& m_application;
  QLabel *m_labelAz = nullptr;
  QLabel *m_labelEl= nullptr;
  QLineEdit *m_editAz= nullptr;
  QLineEdit *m_editEl= nullptr;
  QPushButton *m_buttonGo= nullptr;
  QPushButton *m_buttonStop= nullptr;
};
