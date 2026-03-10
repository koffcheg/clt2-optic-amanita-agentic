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

class WGoto : public QWidget {
  Q_OBJECT
public:
  WGoto(QWidget *parent, turret::Application &application);
  virtual ~WGoto();

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
