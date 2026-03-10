#pragma once

#include "application.hpp"
#include <QCheckBox>
#include <QFileDialog>
#include <QFont>
#include <QLabel>
#include <QLineEdit>
#include <QPen>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <memory>
#include <vector>

class WLeftPanel : public QWidget {
  Q_OBJECT
public:
  WLeftPanel(QWidget *parent, turret::Application &application);
  virtual ~WLeftPanel();
  void updateState();

  bool trackActive() const { return m_track->isChecked(); }
  void setTrackActive(bool active);
private slots:
    void openFileDlg();
signals:
    void onFileOpened(const std::string& filename);    
    void onReset();    
protected:
private:
  turret::Application &m_application;
  QPushButton *m_openBtn = nullptr;
  QPushButton *m_resetBtn = nullptr;
  QCheckBox *m_track = nullptr;
};
