#pragma once

#include "application.hpp"
#include "charts/wChart.hpp"
#include "wLeftPanel.hpp"
#include "fileReader.hpp"
#include <QGridLayout>
#include <QLabel>
#include <QPointer>
#include <QSplitter>
#include <QTabWidget>
#include <memory>
#include <QWidget>

class Window : public QWidget {
  Q_OBJECT

public:
  Window(turret::Application &application);

public slots:
  void refresh();
  void reset();
  void openFile(const std::string &filename);

private:
  void showTargets();
  void reloadFile(bool setMinMax=false);

  turret::Application &m_application;
  //std::unique_ptr<>
  FileReader m_fileReader;

  double T = 0;
  double TW = 10;
  bool m_doClear = false;
  WLeftPanel *m_leftPanel = nullptr;
  WChart *m_chart = nullptr;

  QGridLayout *m_layout = nullptr;
  // QSplitter *m_errorSplitter = nullptr;
  QTimer *m_timer = nullptr;

  std::string m_fileName;
};
