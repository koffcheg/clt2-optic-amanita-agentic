#include "wTrack.hpp"
// #include <QPainter>
#include <cmath>
#include <fmt/format.h>
#include <utils.hpp>

/*
Az: [  ]
El: [  ]

*/

using namespace u;

WTrack::WTrack(QWidget *parent)
    : QWidget(parent) {
  QGridLayout *layout = new QGridLayout(this);

  m_labelAz = new QLabel("Az:", this);
  m_editAz = new QLineEdit("0", this);
  m_labelEl = new QLabel("El:", this);
  m_editEl = new QLineEdit("0", this);
  m_buttonReset = new QPushButton("Reset", this);

  layout->addWidget(m_labelAz, 0, 0);
  layout->addWidget(m_editAz, 0, 1);
  layout->addWidget(m_labelEl, 1, 0);
  layout->addWidget(m_editEl, 1, 1);

  layout->addWidget(m_buttonReset, 0, 2);

  connect(m_editAz, &QLineEdit::editingFinished, this, &WTrack::apply);
  connect(m_editEl, &QLineEdit::editingFinished, this, &WTrack::apply);
  
  connect(m_buttonReset, &QPushButton::clicked, this, &WTrack::onResetClicked);

  setLayout(layout);
}


void WTrack::apply() {
  m_Az = stoddef(m_editAz->text().toStdString(), 0) * DEG;
  m_El = stoddef(m_editEl->text().toStdString(), 0) * DEG;
  emit onApply();
}

WTrack::~WTrack(){};

void WTrack::onResetClicked() {
  
}
