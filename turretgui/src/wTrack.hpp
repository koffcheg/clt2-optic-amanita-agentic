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

class WTrack : public QWidget {
  Q_OBJECT
public:
  WTrack(QWidget *parent);
  virtual ~WTrack();

  double AzDelta() { return m_Az; }
  double ElDelta() { return m_El; }

signals:
  void onApply();

protected:
private:
  void apply();
  void onResetClicked();

  QLabel *m_labelAz = nullptr;
  QLineEdit *m_editAz = nullptr;
  QLabel *m_labelEl = nullptr;
  QLineEdit *m_editEl = nullptr;

  double m_Az = 0;
  double m_El = 0;

  QPushButton *m_buttonReset = nullptr;
};
