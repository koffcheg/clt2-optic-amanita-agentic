#include "wGoto.hpp"
// #include <QPainter>
#include <cmath>
//#include <format>

WGoto::WGoto(QWidget *parent, turret::Application &application)
    : QWidget(parent), m_application(application) {
  QGridLayout *layout = new QGridLayout(this);

  m_labelAz = new QLabel("Az:", this);
  m_labelEl = new QLabel("El:", this);
  m_editAz = new QLineEdit("0", this);
  m_editEl = new QLineEdit("30", this);
  m_buttonGo = new QPushButton("Goto", this);
  m_buttonStop = new QPushButton("Stop", this);

  layout->addWidget(m_labelAz, 0, 0);
  layout->addWidget(m_editAz, 0, 1);
  layout->addWidget(m_labelEl, 1, 0);
  layout->addWidget(m_editEl, 1, 1);
  layout->addWidget(m_buttonGo, 0, 2);
  layout->addWidget(m_buttonStop, 1, 2);

  connect(m_buttonGo, &QPushButton::clicked, this, &WGoto::onGotoClicked);
  connect(m_buttonStop, &QPushButton::clicked, this, &WGoto::onStopClicked);

  setLayout(layout);
}

WGoto::~WGoto(){};

void WGoto::onGotoClicked() {
  double alpha = qDegreesToRadians(m_editAz->text().toDouble());
  double beta = qDegreesToRadians(m_editEl->text().toDouble());
  m_application.controlClient().gotoPos(alpha, beta);
}

void WGoto::onStopClicked() {
    m_application.controlClient().stop();
}
