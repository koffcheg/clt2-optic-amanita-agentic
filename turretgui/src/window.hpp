#pragma once

#include "application.hpp"
#include "charts/wChart.hpp"
#include "charts/wSkyChart.hpp"
#include "wGoto.hpp"
#include "wLeftPanel.hpp"
#include "wSpeed.hpp"
#include "wTrackTest.hpp"
#include "wTrack.hpp"
#include <QGridLayout>
#include <QLabel>
#include <QPointer>
#include <QSplitter>
#include <QTabWidget>
#include <QWidget>

class Window : public QWidget {
  Q_OBJECT

public:
  Window(turret::Application &application);

public slots:
  void refresh();
  void trackTest();
  void onTargetSelected(const SkyTarget* target);
  void onCorrectionApply();

private:
  void showTargets();

  turret::Application &m_application;
  double T = 0;
  double TW = 10;
  bool m_doClear = false;
  WLeftPanel *m_leftPanel = nullptr;
  WSkyChart *m_skyChart = nullptr;
  WChart *m_chartError = nullptr;
  WChart *m_chartSpeed = nullptr;
  QTabWidget *m_tabWidget = nullptr;
  WGoto *m_wGoto = nullptr;
  WSpeed *m_wSpeed = nullptr;
  WTrackTest *m_wTrackTest = nullptr;
  WTrack *m_wTrack = nullptr;
  QGridLayout *m_layout = nullptr;
  QGridLayout *m_layoutRight = nullptr;
  QSplitter *m_errorSplitter = nullptr;
  QTimer *m_timer = nullptr;
  QTimer *m_trackTimer = nullptr;
};
