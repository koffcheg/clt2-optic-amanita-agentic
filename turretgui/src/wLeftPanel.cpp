#include "wLeftPanel.hpp"
#include "theme.hpp"
#include <cmath>

WLeftPanel::WLeftPanel(QWidget *parent, turret::Application &application)
    : QWidget(parent), m_application(application) {

  QGridLayout *layout = new QGridLayout(this);

  m_ledTurretOnline = new WLed(this);
  m_labelTurretOnline = new QLabel("Turret", this);
  m_ledControlOnline = new WLed(this);
  m_labelControlOnline = new QLabel("Control", this);
  m_ledDP2Online = new WLed(this);
  m_labelDP2Online = new QLabel("DP2", this);
  m_ledTop = new WLed(this);
  m_ledBottom = new WLed(this);
  m_ledLeft = new WLed(this);
  m_ledRight = new WLed(this);
  m_stop = new QPushButton("Stop", this);

  int row = 0;
  layout->setAlignment(Qt::AlignTop);
  layout->addWidget(m_ledTurretOnline, row, 0, Qt::AlignLeft);
  layout->addWidget(m_labelTurretOnline, row++, 1, Qt::AlignLeft);
  layout->addWidget(m_ledControlOnline, row, 0, Qt::AlignLeft);
  layout->addWidget(m_labelControlOnline, row++, 1, Qt::AlignLeft);
  layout->addWidget(m_ledDP2Online, row, 0, Qt::AlignLeft);
  layout->addWidget(m_labelDP2Online, row++, 1, Qt::AlignLeft);
  layout->addWidget(m_ledTop, row++, 1, Qt::AlignCenter);
  layout->addWidget(m_ledLeft, row, 1, Qt::AlignLeft);
  layout->addWidget(m_ledRight, row++, 1, Qt::AlignRight);
  layout->addWidget(m_ledBottom, row++, 1, Qt::AlignCenter);
  layout->addWidget(m_stop, row++, 0, 1, 2, Qt::AlignLeft);

  for (int r = 0; r < layout->rowCount(); r++) {
    layout->setRowMinimumHeight(r, 12);
  }

  layout->setColumnMinimumWidth(0, 13);
  layout->setColumnStretch(1, 1);
  layout->setHorizontalSpacing(2);
  layout->setVerticalSpacing(2);

  // m_buttonGo = new QPushButton("Goto", this);
  connect(m_stop, &QPushButton::clicked, this, &WLeftPanel::onStop);

  setLayout(layout);
  // setFixedWidth(100);
}

WLeftPanel::~WLeftPanel(){

};

void WLeftPanel::onStop() { m_application.stop(); }

void WLeftPanel::updateState() {
  if (m_application.infoClient().stateValid()) {
    m_ledTurretOnline->setColor(theme.ledOk());
  } else {
    m_ledTurretOnline->setColor(theme.ledNA());
    m_ledLeft->setColor(theme.ledNA());
    m_ledRight->setColor(theme.ledNA());
    m_ledTop->setColor(theme.ledNA());
    m_ledBottom->setColor(theme.ledNA());
  }
  if (m_application.infoClient().stateValid() &&
      m_application.controlClient().connected()) {
    m_ledControlOnline->setColor(theme.ledOk());
  } else {
    m_ledControlOnline->setColor(theme.ledNA());
  }

  if (false) { // todo: DP2 connection?
    m_ledDP2Online->setColor(theme.ledOk());
  } else {
    m_ledDP2Online->setColor(theme.ledOkOff());
  }

  auto &state = m_application.infoClient().state();

  if (state.alpha.minLimit) {
    m_ledLeft->setColor(theme.ledError());
  } else {
    m_ledLeft->setColor(theme.ledErrorOff());
  }
  if (state.alpha.maxLimit) {
    m_ledRight->setColor(theme.ledError());
  } else {
    m_ledRight->setColor(theme.ledErrorOff());
  }
  if (state.beta.minLimit) {
    m_ledBottom->setColor(theme.ledError());
  } else {
    m_ledBottom->setColor(theme.ledErrorOff());
  }
  if (state.beta.maxLimit) {
    m_ledTop->setColor(theme.ledError());
  } else {
    m_ledTop->setColor(theme.ledErrorOff());
  }
}
