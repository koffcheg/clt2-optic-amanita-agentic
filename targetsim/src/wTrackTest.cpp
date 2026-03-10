#include "wTrackTest.hpp"
// #include <QPainter>
#include <cmath>
#include <fmt/format.h>
#include <utils.hpp>

/*
Az: p0: [  ] A: [  ] Phase:[   ] T:[] V: [  ] a: []
El: p0: [  ] A: [  ] Phase:[   ] T:[] V: [  ] a: []

*/

using namespace u;

WTrackTest::WTrackTest(QWidget *parent) : QWidget(parent) {
  QGridLayout *layout = new QGridLayout(this);

  m_labelAz = new QLabel("Az:", this);
  m_labelAzp0 = new QLabel("p0, °:", this);
  m_labelAzA = new QLabel("A, °:", this);
  m_labelAzPh = new QLabel("ph, °:", this);
  m_labelAzT = new QLabel("T, s:", this);
  m_labelAzV = new QLabel("V, °/s:", this);
  m_labelAza = new QLabel("a, °/s²:", this);
  m_editAzp0 = new QLineEdit("0", this);
  m_editAzA = new QLineEdit("5", this);
  m_editAzPh = new QLineEdit("0", this);
  m_editAzT = new QLineEdit("10", this);
  m_editAzV = new QLineEdit("", this);
  m_editAza = new QLineEdit("", this);
  m_labelEl = new QLabel("El:", this);
  m_labelElp0 = new QLabel("p0, °:", this);
  m_labelElA = new QLabel("A, °:", this);
  m_labelElPh = new QLabel("ph,°:", this);
  m_labelElT = new QLabel("T, s:", this);
  m_labelElV = new QLabel("V, °/s:", this);
  m_labelEla = new QLabel("a, °/s²:", this);
  m_editElp0 = new QLineEdit("0", this);
  m_editElA = new QLineEdit("0", this);
  m_editElPh = new QLineEdit("45", this);
  m_editElT = new QLineEdit("10", this);
  m_editElV = new QLineEdit("", this);
  m_editEla = new QLineEdit("", this);
  m_buttonGo = new QPushButton("Start", this);
  m_buttonStop = new QPushButton("Stop", this);

  layout->addWidget(m_labelAz, 0, 0);
  layout->addWidget(m_labelAzp0, 0, 1);
  layout->addWidget(m_editAzp0, 0, 2);
  layout->addWidget(m_labelAzA, 0, 3);
  layout->addWidget(m_editAzA, 0, 4);
  layout->addWidget(m_labelAzPh, 0, 5);
  layout->addWidget(m_editAzPh, 0, 6);
  layout->addWidget(m_labelAzT, 0, 7);
  layout->addWidget(m_editAzT, 0, 8);
  layout->addWidget(m_labelAzV, 0, 9);
  layout->addWidget(m_editAzV, 0, 10);
  layout->addWidget(m_labelAza, 0, 11);
  layout->addWidget(m_editAza, 0, 12);

  layout->addWidget(m_labelEl, 1, 0);
  layout->addWidget(m_labelElp0, 1, 1);
  layout->addWidget(m_editElp0, 1, 2);
  layout->addWidget(m_labelElA, 1, 3);
  layout->addWidget(m_editElA, 1, 4);
  layout->addWidget(m_labelElPh, 1, 5);
  layout->addWidget(m_editElPh, 1, 6);
  layout->addWidget(m_labelElT, 1, 7);
  layout->addWidget(m_editElT, 1, 8);
  layout->addWidget(m_labelElV, 1, 9);
  layout->addWidget(m_editElV, 1, 10);
  layout->addWidget(m_labelEla, 1, 11);
  layout->addWidget(m_editEla, 1, 12);

  layout->addWidget(m_buttonGo, 0, 13);
  layout->addWidget(m_buttonStop, 1, 13);

  connect(m_editAzp0, &QLineEdit::editingFinished, this,
          &WTrackTest::calcSpeed);
  connect(m_editAzA, &QLineEdit::editingFinished, this, &WTrackTest::calcSpeed);
  connect(m_editAzPh, &QLineEdit::editingFinished, this,
          &WTrackTest::calcSpeed);
  connect(m_editAzT, &QLineEdit::editingFinished, this, &WTrackTest::calcSpeed);
  connect(m_editAzV, &QLineEdit::editingFinished, this, &WTrackTest::calcAmp);
  connect(m_editAza, &QLineEdit::editingFinished, this, &WTrackTest::calcAmp);
  connect(m_editElp0, &QLineEdit::editingFinished, this,
          &WTrackTest::calcSpeed);
  connect(m_editElA, &QLineEdit::editingFinished, this, &WTrackTest::calcSpeed);
  connect(m_editElPh, &QLineEdit::editingFinished, this,
          &WTrackTest::calcSpeed);
  connect(m_editElT, &QLineEdit::editingFinished, this, &WTrackTest::calcSpeed);
  connect(m_editElV, &QLineEdit::editingFinished, this, &WTrackTest::calcAmp);
  connect(m_editEla, &QLineEdit::editingFinished, this, &WTrackTest::calcAmp);

  connect(m_buttonGo, &QPushButton::clicked, this, &WTrackTest::onGotoClicked);
  connect(m_buttonStop, &QPushButton::clicked, this,
          &WTrackTest::onStopClicked);

  setLayout(layout);
  calcSpeed();
}

void WTrackTest::toDouble() {
  m_Azp0 = stoddef(m_editAzp0->text().toStdString(), 0) * DEG;
  m_AzA = stoddef(m_editAzA->text().toStdString(), 0) * DEG;
  m_AzPh = stoddef(m_editAzPh->text().toStdString(), 0) * DEG;
  m_AzT = stoddef(m_editAzT->text().toStdString(), 0);
  m_AzV = stoddef(m_editAzV->text().toStdString(), 0) * DEG;
  m_Aza = stoddef(m_editAza->text().toStdString(), 0) * DEG;
  m_Elp0 = stoddef(m_editElp0->text().toStdString(), 0) * DEG;
  m_ElA = stoddef(m_editElA->text().toStdString(), 0) * DEG;
  m_ElPh = stoddef(m_editElPh->text().toStdString(), 0) * DEG;
  m_ElT = stoddef(m_editElT->text().toStdString(), 0);
  m_ElV = stoddef(m_editElV->text().toStdString(), 0) * DEG;
  m_Ela = stoddef(m_editEla->text().toStdString(), 0) * DEG;
}

void WTrackTest::toString() {
  using namespace fmt;
  m_editAzp0->setText(QString::fromStdString(format("{:.2f}", m_Azp0 / DEG)));
  m_editAzA->setText(QString::fromStdString(format("{:.2f}", m_AzA / DEG)));
  m_editAzPh->setText(QString::fromStdString(format("{:.2f}", m_AzPh / DEG)));
  m_editAzT->setText(QString::fromStdString(format("{:.2f}", m_AzT)));
  m_editAzV->setText(QString::fromStdString(format("{:.2f}", m_AzV / DEG)));
  m_editAza->setText(QString::fromStdString(format("{:.2f}", m_Aza / DEG)));
  m_editElp0->setText(QString::fromStdString(format("{:.2f}", m_Elp0 / DEG)));
  m_editElA->setText(QString::fromStdString(format("{:.2f}", m_ElA / DEG)));
  m_editElPh->setText(QString::fromStdString(format("{:.2f}", m_ElPh / DEG)));
  m_editElT->setText(QString::fromStdString(format("{:.2f}", m_ElT)));
  m_editElV->setText(QString::fromStdString(format("{:.2f}", m_ElV / DEG)));
  m_editEla->setText(QString::fromStdString(format("{:.2f}", m_Ela / DEG)));
}

void WTrackTest::calcSpeed() {
  toDouble();

  m_AzV = m_AzT ? 2 * M_PI * m_AzA / m_AzT : 0;
  m_ElV = m_ElT ? 2 * M_PI * m_ElA / m_ElT : 0;

  m_Aza = m_AzT ? 4 * M_PI * M_PI * m_AzA / m_AzT / m_AzT : 0;
  m_Ela = m_ElT ? 4 * M_PI * M_PI * m_ElA / m_ElT / m_ElT : 0;

  toString();
  doUpdate();
}

void WTrackTest::calcAmp() {
  toDouble();
  m_AzT = m_Aza ? 2 * M_PI * m_AzV / m_Aza : 1;
  m_AzA = m_Aza ? m_AzV * m_AzV / m_Aza : 0;

  m_ElT = m_Ela ? 2 * M_PI * m_ElV / m_Ela : 1;
  m_ElA = m_Ela ? m_ElV * m_ElV / m_Ela : 0;
  toString();
  doUpdate();
}

WTrackTest::~WTrackTest(){};

void WTrackTest::doUpdate() {
  m_trackTest.axes[0].p0 = m_Azp0;
  m_trackTest.axes[0].A = m_AzA;
  m_trackTest.axes[0].Ph = m_AzPh;
  m_trackTest.axes[0].T = m_AzT;
  m_trackTest.axes[1].p0 = m_Elp0;
  m_trackTest.axes[1].A = m_ElA;
  m_trackTest.axes[1].Ph = m_ElPh;
  m_trackTest.axes[1].T = m_ElT;
  emit onUpdate();
}

void WTrackTest::onGotoClicked() {
  doUpdate();
  emit onStart();
}

void WTrackTest::onStopClicked() { emit onStop(); }

void WTrackTest::closeEvent(QCloseEvent *event) {
  QWidget::closeEvent(event);
  emit onClose();
}
