#include "wSpeed.hpp"
// #include <QPainter>
#include <cmath>
//#include <format>

WSpeed::WSpeed(QWidget *parent, turret::Application &application)
    : QWidget(parent), m_application(application) {
  QGridLayout *layout = new QGridLayout(this);

  m_labelAz = new QLabel("Az:", this);
  m_labelEl = new QLabel("El:", this);
  m_editAz = new QLineEdit("5", this);
  m_editEl = new QLineEdit("5", this);
  m_buttonGo = new QPushButton("Speed", this);
  m_buttonStop = new QPushButton("Stop", this);

  layout->addWidget(m_labelAz, 0, 0);
  layout->addWidget(m_editAz, 0, 1);
  layout->addWidget(m_labelEl, 1, 0);
  layout->addWidget(m_editEl, 1, 1);
  layout->addWidget(m_buttonGo, 0, 2);
  layout->addWidget(m_buttonStop, 1, 2);

  connect(m_buttonGo, &QPushButton::clicked, this, &WSpeed::onGotoClicked);
  connect(m_buttonStop, &QPushButton::clicked, this, &WSpeed::onStopClicked);

  setLayout(layout);
}

WSpeed::~WSpeed(){};

void WSpeed::onGotoClicked() {
  double alphaSpeed = qDegreesToRadians(m_editAz->text().toDouble());
  double betaSpeed = qDegreesToRadians(m_editEl->text().toDouble());
  m_application.controlClient().speed(alphaSpeed, betaSpeed);
}

void WSpeed::onStopClicked() {
    m_application.controlClient().stop();
}
