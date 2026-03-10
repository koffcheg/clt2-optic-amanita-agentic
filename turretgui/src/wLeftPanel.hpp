#pragma once

#include "application.hpp"
#include "charts/wLed.hpp"
#include <QCheckBox>
#include <QFont>
#include <QLabel>
#include <QLineEdit>
#include <QPen>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <memory>
#include <vector>

class WLeftPanel : public QWidget {
  Q_OBJECT
public:
  WLeftPanel(QWidget *parent, turret::Application &application);
  virtual ~WLeftPanel();
  void updateState();
private slots:
  void onStop();

protected:
private:
  turret::Application &m_application;

  WLed *m_ledTurretOnline = nullptr;
  QLabel *m_labelTurretOnline = nullptr;
  WLed *m_ledControlOnline = nullptr;
  QLabel *m_labelControlOnline = nullptr;
  WLed *m_ledDP2Online = nullptr;
  QLabel *m_labelDP2Online = nullptr;
  WLed *m_ledTop = nullptr;
  WLed *m_ledBottom = nullptr;
  WLed *m_ledLeft = nullptr;
  WLed *m_ledRight = nullptr;
  QPushButton *m_stop = nullptr;
};
