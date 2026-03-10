#include "wLeftPanel.hpp"
#include "theme.hpp"
#include <cmath>

WLeftPanel::WLeftPanel(QWidget *parent, turret::Application &application)
    : QWidget(parent), m_application(application) {

  QGridLayout *layout = new QGridLayout(this);

  m_openBtn = new QPushButton("Open", this);
  m_resetBtn = new QPushButton("Reset", this);
  connect(m_openBtn, &QPushButton::clicked, this, &WLeftPanel::openFileDlg);
  connect(m_resetBtn, &QPushButton::clicked, this, &WLeftPanel::onReset);
  m_track = new QCheckBox("Track", this);
  m_track->setChecked(true);

  int row = 0;
  layout->setAlignment(Qt::AlignTop);

  layout->addWidget(m_track, row++, 0);
  layout->addWidget(m_openBtn, row++, 0);
  layout->addWidget(m_resetBtn, row++, 0);

  for (int r = 0; r < layout->rowCount(); r++) {
    layout->setRowMinimumHeight(r, 12);
  }

  layout->setColumnMinimumWidth(0, 13);
  layout->setColumnStretch(1, 1);
  layout->setHorizontalSpacing(2);
  layout->setVerticalSpacing(2);

  setLayout(layout);
  setFixedWidth(100);
}

WLeftPanel::~WLeftPanel(){

};

void WLeftPanel::openFileDlg() {
  QString fileName = QFileDialog::getOpenFileName(
      this, "Open File", "", "All Files (*);;Text Files (*.txt *.log)");

  if (!fileName.isEmpty()) {
    emit onFileOpened(fileName.toStdString());
    // fileLabel->setText(fileName);
  }
}

void WLeftPanel::updateState() {}
