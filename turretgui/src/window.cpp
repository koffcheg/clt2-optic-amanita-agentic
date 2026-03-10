#include "window.hpp"
#include "datapro2/src/coordinateTransformation.h"
#include "tg_rc_from_dp2.h"
#include "utils.hpp"
#include <QTimer>

Window::Window(turret::Application &application) : m_application{application} {
  setWindowTitle(tr("Spider Eye: Turret control"));
  m_leftPanel = new WLeftPanel(this, application);

  m_skyChart = new WSkyChart(this);
  m_chartError = new WChart(this);
  m_chartSpeed = new WChart(this);
  m_tabWidget = new QTabWidget(this);
  m_wGoto = new WGoto(this, m_application);
  m_wSpeed = new WSpeed(this, m_application);
  m_wTrackTest = new WTrackTest(this, m_application);
  m_wTrack = new WTrack(this);

  m_errorSplitter = new QSplitter(this);
  m_errorSplitter->setOrientation(Qt::Vertical);

  m_chartError->yAxis().setRight(true);
  m_chartSpeed->yAxis().setRight(true);

  m_layout = new QGridLayout;
  m_layoutRight = new QGridLayout;

  m_errorSplitter->addWidget(m_chartError);
  m_errorSplitter->addWidget(m_chartSpeed);
  m_errorSplitter->addWidget(m_tabWidget);
  m_tabWidget->addTab(m_wGoto, "Goto");
  m_tabWidget->addTab(m_wSpeed, "Speed");
  m_tabWidget->addTab(m_wTrack, "Track");
  m_tabWidget->addTab(m_wTrackTest, "Test");
  m_errorSplitter->addWidget(m_tabWidget);
  m_layoutRight->addWidget(m_errorSplitter, 0, 0);

  m_layout->addWidget(m_leftPanel, 0, 0);
  m_layout->addWidget(m_skyChart, 0, 1);
  m_layout->addLayout(m_layoutRight, 0, 2);
  m_layout->setColumnStretch(1, 1);
  m_layout->setColumnStretch(2, 1);

  setLayout(m_layout);

  m_timer = new QTimer(this);
  connect(m_timer, &QTimer::timeout, this, &Window::refresh);
  m_timer->start(50); // todo: increase period and implement status queue

  m_trackTimer = new QTimer(this);
  connect(m_timer, &QTimer::timeout, this, &Window::trackTest);
  m_trackTimer->start(100); // todo: increase period and implement status queue

  m_skyChart->setPos(NAN, NAN);

  connect(m_skyChart, &WSkyChart::onTargetSelected, this,
          &Window::onTargetSelected);

  connect(m_wTrack, &WTrack::onApply, this, &Window::onCorrectionApply);

  m_chartError->addPlot(std::make_unique<WChartPlot>(*m_chartError));
  m_chartError->addPlot(std::make_unique<WChartPlot>(*m_chartError));
  m_chartError->plots(1).pen().setColor(Qt::blue);
  m_chartError->xAxis().setMinMax(-10, 0);
  m_chartError->yAxis().setMinMax(-0.1, 0.1);

  m_chartSpeed->addPlot(std::make_unique<WChartPlot>(*m_chartSpeed));
  m_chartSpeed->addPlot(std::make_unique<WChartPlot>(*m_chartSpeed));
  m_chartSpeed->plots(1).pen().setColor(Qt::blue);
  m_chartSpeed->xAxis().setMinMax(-10, 0);
  m_chartSpeed->yAxis().setMinMax(-100, 100);

  m_chartError->yAxis().setHighliteZero(true);
  m_chartSpeed->yAxis().setHighliteZero(true);

  resize(1800, 800);

  showMaximized();
}


static void _estimateSpeed(DateTime time, double Az, double El, double &VAz,
                           double &VEl) {
  
  static struct {
    double vAz = 0;
    double vEl = 0;
    double lastAz = 0;
    double lastEl = 0;
    DateTime lastT{};
  } tc;

  double dt =
      std::chrono::duration<double>(time - tc.lastT).count();
  if (dt < 1) { // less than a second
    double vAz = (Az - tc.lastAz) / dt;
    double vEl = (El - tc.lastEl) / dt;
    double tau = 1;
    vAz = (tc.vAz*tau+vAz)/(tau+1);
    vEl = (tc.vEl*tau+vEl)/(tau+1);
        tc.vAz = vAz;
    tc.vEl = vEl;
  }

  VAz = tc.vAz;
  VEl = tc.vEl;
  tc.lastT = time;
  tc.lastAz = Az;
  tc.lastEl = El;
}

void Window::showTargets() {
  auto tracks = get_curr_tracks_from_dp2();
  // simple clear and fill aproach
  // todo: optimize:

  auto selectedTarget = m_skyChart->selectedTarget();
  unsigned int selectedTargetId = selectedTarget ? selectedTarget->id : -1;
  // setWindowTitle(tr(std::to_string(tracks.size()).c_str())); //1

  m_skyChart->targets().clear();
  m_skyChart->trajectories().clear();
  for (auto &dp2 : tracks) {
    // setWindowTitle(tr(std::to_string(dp2.second.size()).c_str()));
    for (auto &trajectory : dp2.second) {
      double Az, El, VAz, VEl;
      auto time = std::chrono::system_clock::now();

      //extrapolation
      //AzEl_V_a(trajectory, time, Az, El, VAz, VEl);


      extrapolationAzEl_4_binocular(trajectory, time, Az, El);
      extrapolationVazVel_4_binocular(trajectory, time, VAz, VEl);
      //_estimateSpeed(time, Az, El, VAz, VEl);


      m_skyChart->targets().push_back(
          SkyTarget{trajectory.id, SkyPosition(Az, El), SkyPosition(VAz, VEl),
                    trajectory.id == selectedTargetId, "label", Qt::darkGray});

      // SkyTrajectory skyTrajectory;
      // skyTrajectory.color = Qt::darkGray;
      // skyTrajectory.label = "label";
      // skyTrajectory.selected = false;
      // skyTrajectory.visible = true;
      ////for (auto &measurement : trajectory.measurements) {
      ////  skyTrajectory.path.push_back(
      ////      SkyPosition(measurement.x, measurement.y)); // measurement.time
      ////}
      // m_skyChart.trajectories()[trajectory.id] = skyTrajectory;
    }
  }
  // if ()
}

void Window::trackTest() {
  m_application.processTrackTest();
  if (m_application.trackTestEnabled()) {
    auto p = m_application.trackTestPos();
    m_skyChart->targets().clear();
    m_skyChart->trajectories().clear();
    m_skyChart->targets().push_back(SkyTarget{
        0, SkyPosition(p.alpha, p.beta), SkyPosition(p.alphaSpeed, p.betaSpeed),
        true, "test", Qt::darkGray});
  }
}

void Window::refresh() {
  if (!m_application.infoClient().stateValid()) {
    m_skyChart->setPos(NAN, NAN);
  } else {
    const turret::TurretState &state = m_application.infoClient().state();

    m_skyChart->setPos(state.alpha.position, state.beta.position);

    if (state.axes[0].mode != turret::AxisMode::stopped ||
        state.axes[1].mode != turret::AxisMode::stopped) {
      if (m_doClear) {
        m_chartError->plots(0).clear();
        m_chartError->plots(1).clear();
        m_chartSpeed->plots(0).clear();
        m_chartSpeed->plots(1).clear();
        m_doClear = false;
      }
      m_chartError->plots(0).append(T, qRadiansToDegrees(state.axes[0].error));
      m_chartError->plots(1).append(T, qRadiansToDegrees(state.axes[1].error));

      m_chartSpeed->plots(0).append(T, qRadiansToDegrees(state.axes[0].speed));
      m_chartSpeed->plots(1).append(T, qRadiansToDegrees(state.axes[1].speed));

      TW = m_chartError->xAxis().maximum() - m_chartError->xAxis().minimum();
      m_chartError->xAxis().setMinMax(T - TW, T);
      m_chartSpeed->xAxis().setMinMax(T - TW, T);
      T += 0.05; // todo:
    } else {
      T = 0;
      m_doClear = true;
    }
  }
  m_leftPanel->updateState();
  showTargets();
}

void Window::onTargetSelected(const SkyTarget *target) {
  if (target) {
    m_application.startTrackObject(target->id);
  } else {
    if (m_application.trackObject()) {
      m_application.stop();
    }
  }
}

void Window::onCorrectionApply() {
  m_application.setTrackObjectCorrections(m_wTrack->AzDelta(),
                                          m_wTrack->ElDelta());
}
