
#pragma once

#include <QColor>

class Theme {
public:
  const QColor &ledNA() const { return m_ledNA; }
  const QColor &ledOff() const { return m_ledOff; }
  const QColor &ledOnOff() const { return m_ledOnOff; }
  const QColor &ledOkOff() const { return m_ledOkOff; }
  const QColor &ledWarningOff() const { return m_ledWarningOff; }
  const QColor &ledErrorOff() const { return m_ledErrorOff; }
  const QColor &ledActOff() const { return m_ledActOff; }
  const QColor &ledOn() const { return m_ledOn; }
  const QColor &ledOk() const { return m_ledOk; }
  const QColor &ledWarning() const { return m_ledWarning; }
  const QColor &ledError() const { return m_ledError; }
  const QColor &ledAct() const { return m_ledAct; }

private:
  QColor m_ledNA{Qt::darkGray};
  QColor m_ledOff{Qt::darkGray};
  QColor m_ledOnOff{0x60, 0x80, 0x60};    //(Qt::darkGray);//(Qt::darkGreen};
  QColor m_ledOkOff{0x60, 0x80, 0x60};    //(Qt::darkGray);//(Qt::darkGreen};
  QColor m_ledWarningOff{Qt::darkGray};   //(Qt::darkYellow};
  QColor m_ledErrorOff{0x80, 0x60, 0x60}; //(Qt::darkGray);//(Qt::darkRed};
  QColor m_ledActOff{Qt::darkBlue};
  QColor m_ledOn{Qt::green};
  QColor m_ledOk{Qt::green};
  QColor m_ledWarning{Qt::yellow};
  QColor m_ledError{Qt::red};
  QColor m_ledAct{Qt::blue};

}; // namespace theme

extern Theme theme;
