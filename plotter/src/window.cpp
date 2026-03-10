#include "window.hpp"
#include "theme.hpp"
#include "utils.hpp"

#include <QTimer>
#include <fmt/format.h>

Window::Window(turret::Application &application) : m_application{application} {
  setWindowTitle(tr("Plotter:"));
  m_leftPanel = new WLeftPanel(this, application);
  connect(m_leftPanel, &WLeftPanel::onFileOpened, this, &Window::openFile);
  connect(m_leftPanel, &WLeftPanel::onReset, this, &Window::reset);

  m_chart = new WChart(this);

  m_chart->yAxis().setRight(true);

  m_layout = new QGridLayout;

  m_layout->addWidget(m_leftPanel, 0, 0);
  m_layout->addWidget(m_chart, 0, 1);
  // m_layout->setColumnMinimumWidth(0, 100);
  m_layout->setColumnStretch(1, 1);

  setLayout(m_layout);

  m_timer = new QTimer(this);
  connect(m_timer, &QTimer::timeout, this, &Window::refresh);
  m_timer->start(100); // todo: increase period and implement status queue

  m_chart->xAxis().setMinMax(0, 10);
  m_chart->yAxis().setMinMax(-0.1, 0.1);

  resize(1800, 800);

  showMaximized();

  if (!application.filename().empty()) {
    openFile(application.filename());
  }
}

void Window::refresh() {
  if (!m_fileName.empty()) {
    reloadFile();
  }
}

void Window::reset() {
  m_fileReader.clear();
  m_chart->clearPlots();
  if (!m_fileName.empty()) {
    openFile(m_fileName);
  }
}

void Window::reloadFile(bool setMinMax) {
  m_fileReader.loadFile(m_fileName);
  if (m_fileReader.data().size() == 0) {
    return;
  }

  auto t0 = m_fileReader.data()[0].t;
  double t = 10;
  double Min = INFINITY;
  double Max = -INFINITY;
  size_t wasLoaded = 0;

  if (m_chart->plots().size()) {
    wasLoaded = m_chart->plots(0).size();
    if (m_chart->plots(0).size()) {
      t = m_chart->plots(0)[m_chart->plots(0).size() - 1].x;
    }
  }
  bool trackRequest = wasLoaded != m_fileReader.data().size();
  for (size_t i = wasLoaded; i < m_fileReader.data().size(); i++) {
    auto &row = m_fileReader.data()[i];
    t = u::toSeconds(row.t - t0);
    for (size_t j = 0; j < row.measures.size(); j++) {
      double y = row.measures[j];
      m_chart->plots(j).append(t, y);

      if (y > Max) {
        Max = y;
      }
      if (y < Min) {
        Min = y;
      }
    }
  }

  if (setMinMax) {
    if (Min==Max || isinf(Min) || isinf(Max)) {
      Min = -1;
      Max = 1;
    }
    m_chart->xAxis().setMinMax(0, t);
    m_chart->yAxis().setMinMax(Min, Max);
  } else {
    if (m_leftPanel->trackActive() && trackRequest) {
      TW = m_chart->xAxis().maximum()-m_chart->xAxis().minimum();
      m_chart->xAxis().setMinMax(t - TW, t);
    }
  }
}

void Window::openFile(const std::string &filename) {
  m_fileReader.clear();
  m_fileReader.loadFile(filename);
  constexpr size_t skipCols = 2; //date and time!
  m_chart->clearPlots();
  for (size_t i = skipCols; i < m_fileReader.columns().size(); i++) {
    m_chart->addPlot(std::make_unique<WChartPlot>(
        *m_chart, QString::fromStdString(m_fileReader.columns()[i])));
    m_chart->plots(i - skipCols).pen().setColor(theme.plotColor(i - skipCols));
  }
  m_fileName = filename;

  reloadFile(true);

  setWindowTitle(tr(fmt::format("Plotter: {}", m_fileName).c_str()));
}
