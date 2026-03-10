#pragma once

#include "application.hpp"
#include "trackTest.hpp"
#include <QFont>
#include <QLabel>
#include <QLineEdit>
#include <QPen>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <memory>
#include <vector>

class WTrackTest : public QWidget {
  Q_OBJECT
public:
  WTrackTest(QWidget *parent, turret::Application &application);
  virtual ~WTrackTest();

protected:
private:
  void toDouble();
  void toString();
  void calcSpeed();
  void calcAmp();
  void onGotoClicked();
  void onStopClicked();

  turret::Application &m_application;
  QLabel *m_labelAz = nullptr;
  QLabel *m_labelAzp0 = nullptr;
  QLabel *m_labelAzA = nullptr;
  QLabel *m_labelAzPh = nullptr;
  QLabel *m_labelAzT = nullptr;
  QLabel *m_labelAzV = nullptr;
  QLabel *m_labelAza = nullptr;
  QLineEdit *m_editAzp0 = nullptr;
  QLineEdit *m_editAzA = nullptr;
  QLineEdit *m_editAzPh = nullptr;
  QLineEdit *m_editAzT = nullptr;
  QLineEdit *m_editAzV = nullptr;
  QLineEdit *m_editAza = nullptr;
  QLabel *m_labelEl = nullptr;
  QLabel *m_labelElp0 = nullptr;
  QLabel *m_labelElA = nullptr;
  QLabel *m_labelElPh = nullptr;
  QLabel *m_labelElT = nullptr;
  QLabel *m_labelElV = nullptr;
  QLabel *m_labelEla = nullptr;
  QLineEdit *m_editElp0 = nullptr;
  QLineEdit *m_editElA = nullptr;
  QLineEdit *m_editElPh = nullptr;
  QLineEdit *m_editElT = nullptr;
  QLineEdit *m_editElV = nullptr;
  QLineEdit *m_editEla = nullptr;


  TrackTest m_trackTest;

  double m_Azp0 = 0;
  double m_AzA = 0;
  double m_AzPh = 0;
  double m_AzT = 0;
  double m_AzV = 0;
  double m_Aza = 0;
  double m_Elp0 = 0;
  double m_ElA = 0;
  double m_ElPh = 0;
  double m_ElT = 0;
  double m_ElV = 0;
  double m_Ela = 0;

  QPushButton *m_buttonGo = nullptr;
  QPushButton *m_buttonStop = nullptr;
};
