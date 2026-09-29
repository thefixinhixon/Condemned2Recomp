#include "main_window.h"

#include "import_service.h"
#include "keybind_button.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QProcessEnvironment>
#include <QProgressDialog>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QStandardPaths>
#include <QTabWidget>
#include <QTextStream>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

namespace condemned2::launcher {
namespace {

QComboBox* valueCombo(
    std::initializer_list<std::pair<QString, QVariant>> entries) {
  auto* result = new QComboBox;
  for (const auto& [label, value] : entries) {
    result->addItem(label, value);
  }
  return result;
}

void selectValue(QComboBox* combo, const QVariant& value) {
  const int index = combo->findData(value);
  combo->setCurrentIndex(index >= 0 ? index : 0);
}

QGroupBox* group(const QString& title, QLayout* layout) {
  auto* box = new QGroupBox(title);
  box->setLayout(layout);
  return box;
}

}  

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
#if defined(__aarch64__)
  setWindowTitle(QStringLiteral("Condemned 2: Bloodshot — ARM64"));
#elif defined(__x86_64__)
  setWindowTitle(QStringLiteral("Condemned 2: Bloodshot — x86_64 Linux"));
#else
  setWindowTitle(QStringLiteral("Condemned 2: Bloodshot"));
#endif
  resize(920, 650);
  setMinimumSize(760, 560);

  auto* tabs = new QTabWidget;
  tabs->addTab(createPlayPage(), tr("PLAY"));
  tabs->addTab(createGraphicsPage(), tr("GRAPHICS"));
  tabs->addTab(createAudioPage(), tr("AUDIO"));
  tabs->addTab(createControlsPage(), tr("CONTROLS"));
  tabs->addTab(createLocalePage(), tr("LANGUAGE"));
  setCentralWidget(tabs);

  setStyleSheet(QStringLiteral(R"(
    QMainWindow, QWidget { background: #160808; color: #f0e4dc; }
    QTabWidget::pane, QGroupBox { border: 1px solid #603020; }
    QTabBar::tab { background: #25100c; padding: 10px 18px; }
    QTabBar::tab:selected { background: #7a250c; }
    QPushButton { background: #8f2d0d; border: 1px solid #c65322;
                  padding: 8px 14px; border-radius: 3px; }
    QPushButton:hover { background: #ad3a12; }
    QPushButton:disabled { background: #3b2822; color: #89766e; }
    QComboBox, QSpinBox { background: #251712; padding: 5px; }
    QGroupBox { margin-top: 12px; padding-top: 12px; font-weight: bold; }
    QGroupBox::title { subcontrol-origin: margin; left: 10px; }
  )"));

  dataRoot_ = GameConfig::selectedDataRoot();
  gameRoot_ = GameConfig::selectedGameRoot();

  if (!dataRoot_.isEmpty()) {
    const QString testFile =
        QDir(dataRoot_).filePath(QStringLiteral(".__condemned2_write_test__"));
    QFile f(testFile);
    if (!f.open(QIODevice::WriteOnly)) {
      dataRoot_.clear();
    } else {
      f.close();
      QFile::remove(testFile);
    }
  }

  if (!dataRoot_.isEmpty()) {
    loadSettings();
  } else {
    applySettings(GameSettings{});
  }
  updateStatus();

  QTimer::singleShot(0, this, [this] {
    if (dataRoot_.isEmpty() || gameRoot_.isEmpty()) {
      runFirstSetup();
    }
  });
}

QWidget* MainWindow::createPlayPage() {
  auto* page = new QWidget;
  auto* layout = new QVBoxLayout(page);
  layout->setContentsMargins(36, 32, 36, 32);

  auto* title = new QLabel(tr("CONDEMNED 2: BLOODSHOT"));
  QFont titleFont = title->font();
  titleFont.setPointSize(26);
  titleFont.setBold(true);
  title->setFont(titleFont);
  title->setAlignment(Qt::AlignCenter);
  layout->addWidget(title);

  statusLabel_ = new QLabel;
  statusLabel_->setWordWrap(true);
  statusLabel_->setAlignment(Qt::AlignCenter);
  layout->addWidget(statusLabel_);

  dataRootLabel_ = new QLabel;
  dataRootLabel_->setWordWrap(true);
  dataRootLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
  dataRootLabel_->setAlignment(Qt::AlignCenter);
  layout->addWidget(dataRootLabel_);

  gameRootLabel_ = new QLabel;
  gameRootLabel_->setWordWrap(true);
  gameRootLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
  gameRootLabel_->setAlignment(Qt::AlignCenter);
  layout->addWidget(gameRootLabel_);
  layout->addStretch();

  playButton_ = new QPushButton(tr("PLAY"));
  playButton_->setMinimumSize(230, 80);
  QFont playFont = playButton_->font();
  playFont.setPointSize(24);
  playFont.setBold(true);
  playButton_->setFont(playFont);
  connect(playButton_, &QPushButton::clicked, this, &MainWindow::play);
  layout->addWidget(playButton_, 0, Qt::AlignCenter);

  auto* actions = new QHBoxLayout;
  auto* dataRootButton = new QPushButton(tr("Change save data"));
  auto* gameRootButton = new QPushButton(tr("Change game folder"));
  importIsoButton_ = new QPushButton(tr("Import ISO"));
  auto* logsButton = new QPushButton(tr("Open logs"));
  connect(dataRootButton, &QPushButton::clicked, this,
          &MainWindow::chooseDataRoot);
  connect(gameRootButton, &QPushButton::clicked, this,
          &MainWindow::chooseGameRoot);
  connect(importIsoButton_, &QPushButton::clicked, this, &MainWindow::importIso);
  connect(logsButton, &QPushButton::clicked, this, &MainWindow::openLogs);
  actions->addStretch();
  actions->addWidget(dataRootButton);
  actions->addWidget(gameRootButton);
  actions->addWidget(importIsoButton_);
  actions->addWidget(logsButton);
  actions->addStretch();
  layout->addLayout(actions);

  auto* version =
#if defined(__aarch64__)
      new QLabel(tr("Launcher ARM64 %1").arg(QStringLiteral(CONDEMNED2_LAUNCHER_VERSION)));
#elif defined(__x86_64__)
      new QLabel(tr("Launcher x86_64 Linux %1").arg(QStringLiteral(CONDEMNED2_LAUNCHER_VERSION)));
#else
      new QLabel(tr("Launcher %1").arg(QStringLiteral(CONDEMNED2_LAUNCHER_VERSION)));
#endif
  version->setAlignment(Qt::AlignRight);
  layout->addWidget(version);
  return page;
}

QWidget* MainWindow::createGraphicsPage() {
  auto* page = new QWidget;
  auto* outer = new QVBoxLayout(page);
  auto* form = new QFormLayout;

  resolutionScale_ =
      valueCombo({{tr("720p (1×)"), 1}, {tr("1440p (2×)"), 2},
                  {tr("2160p (3×)"), 3}});
  antiAliasing_ =
      valueCombo({{tr("Off"), QStringLiteral("none")},
                  {QStringLiteral("FXAA"), QStringLiteral("fxaa")},
                  {tr("FXAA Extreme"), QStringLiteral("fxaa_extreme")}});
  upscaling_ =
      valueCombo({{tr("Bilinear (default)"), QStringLiteral("bilinear")},
                  {QStringLiteral("AMD FSR"), QStringLiteral("fsr")}});
  anisotropic_ =
      valueCombo({{tr("Default"), -1}, {QStringLiteral("2×"), 2},
                  {QStringLiteral("4×"), 4}, {QStringLiteral("8×"), 8},
                  {QStringLiteral("16×"), 16}});
  pipelineThreads_ = new QSpinBox;
  pipelineThreads_->setRange(-1, 16);
  pipelineThreads_->setSpecialValueText(tr("Automatic"));

  form->addRow(tr("Resolution scale:"), resolutionScale_);
  form->addRow(tr("Anti-aliasing:"), antiAliasing_);
  form->addRow(tr("Upscaling:"), upscaling_);
  form->addRow(tr("Anisotropic filtering:"), anisotropic_);
  form->addRow(tr("Vulkan pipeline threads:"), pipelineThreads_);
  outer->addWidget(group(tr("Graphics quality"), form));

  auto* flags = new QVBoxLayout;
  fullscreen_ = new QCheckBox(tr("Fullscreen"));
  vsync_ = new QCheckBox(tr("Vertical sync"));
  nativeMsaa_ = new QCheckBox(tr("Native 2× MSAA"));
  asyncShaders_ = new QCheckBox(tr("Async shader compilation"));
  flags->addWidget(fullscreen_);
  flags->addWidget(vsync_);
  flags->addWidget(nativeMsaa_);
  flags->addWidget(asyncShaders_);
  auto* frameLimitRow = new QHBoxLayout;
  frameLimitRow->addWidget(new QLabel(tr("Frame rate limit:")));
  frameLimit_ = new QSpinBox;
  frameLimit_->setRange(0, 240);
  frameLimit_->setSpecialValueText(tr("Unlimited"));
  frameLimit_->setSuffix(tr(" FPS"));
  frameLimitRow->addWidget(frameLimit_);
  frameLimitRow->addStretch();
  flags->addLayout(frameLimitRow);
  outer->addWidget(group(tr("Presentation and performance"), flags));

  auto* note = new QLabel(tr(
#if defined(Q_PROCESSOR_ARM_64)
      "The Vulkan FBO backend and disabled sparse memory are applied "
      "automatically for ARM64/Turnip."
#else
      "Running on x86_64: default Vulkan render target path and sparse "
      "shared memory are used. ARM64/Turnip builds force the FBO backend "
      "and disable sparse memory automatically."
#endif
      ));
  note->setWordWrap(true);
  outer->addWidget(note);
  outer->addStretch();

  auto* buttons = new QHBoxLayout;
  auto* reset = new QPushButton(tr("Recommended values"));
  auto* save = new QPushButton(tr("Save"));
  connect(reset, &QPushButton::clicked, this, &MainWindow::resetSettings);
  connect(save, &QPushButton::clicked, this, &MainWindow::saveSettings);
  buttons->addStretch();
  buttons->addWidget(reset);
  buttons->addWidget(save);
  outer->addLayout(buttons);
  return page;
}

QWidget* MainWindow::createAudioPage() {
  auto* page = new QWidget;
  auto* outer = new QVBoxLayout(page);

  auto* flags = new QVBoxLayout;
  forceStereo_ = new QCheckBox(tr("Force stereo output"));
  xmaWorker_ = new QCheckBox(tr("Use dedicated XMA worker"));
  frontOnly_ = new QCheckBox(tr("Use front channels only"));
  flags->addWidget(forceStereo_);
  flags->addWidget(xmaWorker_);
  flags->addWidget(frontOnly_);
  outer->addWidget(group(tr("Processing"), flags));

  auto* form = new QFormLayout;
  queuedFrames_ = new QSpinBox;
  queuedFrames_->setRange(16, 128);
  highpassHz_ = new QSpinBox;
  highpassHz_->setRange(0, 500);
  highpassHz_->setSuffix(QStringLiteral(" Hz"));
  outputGain_ = new QSlider(Qt::Horizontal);
  outputGain_->setRange(10, 100);
  outputGainLabel_ = new QLabel;
  auto* gainRow = new QHBoxLayout;
  gainRow->addWidget(outputGain_);
  gainRow->addWidget(outputGainLabel_);
  connect(outputGain_, &QSlider::valueChanged, this, [this](int value) {
    outputGainLabel_->setText(QStringLiteral("%1%").arg(value));
  });
  form->addRow(tr("Queued frames:"), queuedFrames_);
  form->addRow(tr("Output gain:"), gainRow);
  form->addRow(tr("High-pass filter:"), highpassHz_);
  outer->addWidget(group(tr("Output"), form));

  auto* note = new QLabel(tr(
      "Disabling the XMA worker serializes decoding. That is the "
      "recommended value for this ARM64 build."));
  note->setWordWrap(true);
  outer->addWidget(note);
  outer->addStretch();

  auto* save = new QPushButton(tr("Save"));
  connect(save, &QPushButton::clicked, this, &MainWindow::saveSettings);
  outer->addWidget(save, 0, Qt::AlignRight);
  return page;
}

QWidget* MainWindow::createLocalePage() {
  auto* page = new QWidget;
  auto* outer = new QVBoxLayout(page);
  auto* form = new QFormLayout;

  language_ = new QComboBox;
  const struct {
    const char* label;
    int language;
    int country;
  } locales[] = {
      {"English", 1, 103}, {"日本語", 2, 53},   {"Deutsch", 3, 24},
      {"Français", 4, 34}, {"Español", 5, 31}, {"Italiano", 6, 50},
      {"한국어", 7, 56},   {"中文", 8, 20},     {"Português", 9, 84},
      {"Polski", 11, 82},  {"Русский", 12, 88},
  };
  for (const auto& locale : locales) {
    language_->addItem(QString::fromUtf8(locale.label), locale.language);
    language_->setItemData(language_->count() - 1, locale.country,
                           Qt::UserRole + 1);
  }
  country_ = new QSpinBox;
  country_->setRange(0, 255);
  logging_ = new QCheckBox(tr("Enable diagnostic logs"));
  connect(language_, &QComboBox::currentIndexChanged, this, [this](int index) {
    country_->setValue(language_->itemData(index, Qt::UserRole + 1).toInt());
  });

  form->addRow(tr("Language (Language ID):"), language_);
  form->addRow(tr("Country (Country ID):"), country_);
  form->addRow(QString(), logging_);
  outer->addWidget(group(tr("Xbox 360 system"), form));

  auto* note = new QLabel(tr(
      "The game must contain files for the selected language. Spanish uses "
      "Language ID 5 and Country ID 31."));
  note->setWordWrap(true);
  outer->addWidget(note);
  outer->addStretch();

  auto* save = new QPushButton(tr("Save"));
  connect(save, &QPushButton::clicked, this, &MainWindow::saveSettings);
  outer->addWidget(save, 0, Qt::AlignRight);
  return page;
}

QWidget* MainWindow::createControlsPage() {
  auto* page = new QWidget;
  auto* outer = new QVBoxLayout(page);

  auto* backendForm = new QFormLayout;
  inputBackend_ = valueCombo({{tr("SDL"), QStringLiteral("sdl")}});
  guideButton_ = new QCheckBox(tr("Enable guide button pass-through"));
  backendForm->addRow(tr("Input backend:"), inputBackend_);
  backendForm->addRow(QString(), guideButton_);
  outer->addWidget(group(tr("Gamepad"), backendForm));

  auto* mnkForm = new QFormLayout;
  mnkMode_ = new QCheckBox(tr("Enable keyboard/mouse controller emulation"));
  mnkMouse_ = new QCheckBox(tr("Use mouse for right stick"));
  mnkSensitivity_ = new QSlider(Qt::Horizontal);
  mnkSensitivity_->setRange(10, 400);
  mnkSensitivityLabel_ = new QLabel;
  auto* sensRow = new QHBoxLayout;
  sensRow->addWidget(mnkSensitivity_);
  sensRow->addWidget(mnkSensitivityLabel_);
  connect(mnkSensitivity_, &QSlider::valueChanged, this, [this](int value) {
    mnkSensitivityLabel_->setText(QStringLiteral("%1%").arg(value));
  });
  mnkForm->addRow(QString(), mnkMode_);
  mnkForm->addRow(QString(), mnkMouse_);
  mnkForm->addRow(tr("Mouse sensitivity:"), sensRow);
  outer->addWidget(group(tr("Keyboard & mouse"), mnkForm));

  // Controller keybinds: label + capture button per bind, in GameSettings order.
  const struct {
    const char* label;
  } binds[] = {
      {"A button"}, {"B button"}, {"X button"}, {"Y button"},
      {"Left trigger"}, {"Right trigger"}, {"Left shoulder"}, {"Right shoulder"},
      {"Left stick up"}, {"Left stick down"}, {"Left stick left"}, {"Left stick right"},
      {"Left stick press"},
      {"Right stick up"}, {"Right stick down"}, {"Right stick left"}, {"Right stick right"},
      {"Right stick press"},
      {"D-pad up"}, {"D-pad down"}, {"D-pad left"}, {"D-pad right"},
      {"Back button"}, {"Start button"}, {"Guide button"},
  };
  auto* bindForm = new QFormLayout;
  keybindButtons_.clear();
  for (const auto& bind : binds) {
    auto* button = new KeybindButton;
    button->setMinimumWidth(220);
    keybindButtons_.append(button);
    bindForm->addRow(tr(bind.label) + QStringLiteral(":"), button);
  }
  auto* bindScroll = new QWidget;
  bindScroll->setLayout(bindForm);
  outer->addWidget(group(tr("Controller keybinds"), new QVBoxLayout));
  // Replace the empty layout with the form inside a scroll area.
  auto* bindGroup = qobject_cast<QGroupBox*>(outer->itemAt(outer->count() - 1)->widget());
  delete bindGroup->layout();
  auto* scroll = new QScrollArea;
  scroll->setWidgetResizable(true);
  scroll->setWidget(bindScroll);
  scroll->setMinimumHeight(220);
  auto* bindLayout = new QVBoxLayout;
  bindLayout->addWidget(scroll);
  bindGroup->setLayout(bindLayout);

  auto* note = new QLabel(tr(
      "Click a keybind, then press a key to rebind it. Bindings are passed to "
      "the game's keyboard/mouse controller emulation."));
  note->setWordWrap(true);
  outer->addWidget(note);
  outer->addStretch();

  auto* save = new QPushButton(tr("Save"));
  connect(save, &QPushButton::clicked, this, &MainWindow::saveSettings);
  outer->addWidget(save, 0, Qt::AlignRight);
  return page;
}

QWidget* MainWindow::createSettingsPage() { return new QWidget; }

bool MainWindow::setDataRoot(const QString& root) {
  dataRoot_ = QDir::cleanPath(QDir(root).absolutePath());
  if (dataRoot_.isEmpty()) {
    return false;
  }

  const QString configDir = QDir(dataRoot_).filePath(QStringLiteral("config"));
  const QString savesDir = QDir(dataRoot_).filePath(QStringLiteral("saves"));
  const QString cacheDir = QDir(dataRoot_).filePath(QStringLiteral("cache"));
  const QString logsDir = QDir(dataRoot_).filePath(QStringLiteral("logs"));

  if (!QDir().mkpath(configDir) || !QDir().mkpath(savesDir) ||
      !QDir().mkpath(cacheDir) || !QDir().mkpath(logsDir)) {
    QMessageBox::critical(this, tr("Path is not writable"),
                          tr("Could not create folders inside: %1")
                              .arg(dataRoot_));
    dataRoot_.clear();
    return false;
  }

  GameConfig::setSelectedDataRoot(dataRoot_);
  loadSettings();
  updateStatus();
  return true;
}

void MainWindow::setGameRoot(const QString& root) {
  gameRoot_ = QDir::cleanPath(QDir(root).absolutePath());
  GameConfig::setSelectedGameRoot(gameRoot_);
  updateStatus();
}

bool MainWindow::runFirstSetup() {
  QMessageBox::information(
      this, tr("First launch"),
      tr("Select the save data folder first, then the game folder. "
         "The AppImage will remember both paths."));
  const QString selectedDataRoot = QFileDialog::getExistingDirectory(
      this, tr("Select save data folder"),
      dataRoot_.isEmpty() ? QDir::homePath() : dataRoot_);
  if (selectedDataRoot.isEmpty()) {
    return false;
  }
  if (!setDataRoot(selectedDataRoot)) {
    return false;
  }

  const QString selectedGameRoot = QFileDialog::getExistingDirectory(
      this, tr("Select game folder"),
      gameRoot_.isEmpty() ? QDir::homePath() : gameRoot_);
  if (selectedGameRoot.isEmpty()) {
    return false;
  }
  setGameRoot(selectedGameRoot);

  if (!ImportService::hasValidGame(gameRoot_)) {
    QMessageBox::information(
        this, tr("Game not imported"),
        tr("The game folder does not contain default.xex yet. Use "
           "\"Import ISO\" or select a folder that already has it."));
  }
  return true;
}

void MainWindow::chooseDataRoot() {
  const QString selected = QFileDialog::getExistingDirectory(
      this, tr("Select save data folder"),
      dataRoot_.isEmpty() ? QDir::homePath() : dataRoot_);
  if (!selected.isEmpty()) {
    setDataRoot(selected);
  }
}

void MainWindow::chooseGameRoot() {
  const QString selected = QFileDialog::getExistingDirectory(
      this, tr("Select game folder"),
      gameRoot_.isEmpty() ? QDir::homePath() : gameRoot_);
  if (!selected.isEmpty()) {
    setGameRoot(selected);
  }
}

bool MainWindow::confirmReplaceGame() {
  if (!ImportService::hasValidGame(gameRoot_)) {
    return true;
  }
  return QMessageBox::question(
             this, tr("Replace files"),
             tr("This folder already contains a valid game. Do you want "
                "to replace it?"),
             QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;
}

bool MainWindow::performImport(const QString& isoPath) {
  if (!confirmReplaceGame()) {
    return false;
  }

  QProgressDialog progress(tr("Preparing import…"), tr("Cancel"), 0,
                           100, this);
  progress.setWindowModality(Qt::WindowModal);
  progress.setMinimumDuration(0);
  ProgressCallback callback =
      [&progress](qint64 completed, qint64 total, const QString& detail) {
        if (total > 0) {
          progress.setRange(0, 1000);
          progress.setValue(
              static_cast<int>((completed * 1000) / qMax<qint64>(1, total)));
        } else {
          progress.setRange(0, 0);
        }
        if (!detail.isEmpty()) {
          progress.setLabelText(detail);
        }
        QApplication::processEvents();
        return !progress.wasCanceled();
      };

  const ImportResult result = ImportService::extractIso(
      isoPath, gameRoot_, ImportService::findExtractor(), callback);
  progress.close();
  if (!result.success && !result.cancelled) {
    QMessageBox::critical(this, tr("Import error"), result.error);
  } else if (result.success) {
    QMessageBox::information(this, tr("Import complete"),
                             tr("Game files are ready."));
  }
  updateStatus();
  return result.success;
}

void MainWindow::importIso() {
  if (dataRoot_.isEmpty() || gameRoot_.isEmpty()) {
    runFirstSetup();
    if (dataRoot_.isEmpty() || gameRoot_.isEmpty()) {
      return;
    }
  }
  const QString iso = QFileDialog::getOpenFileName(
      this, tr("Select ISO"), QDir::homePath(),
      tr("Xbox 360 ISO (*.iso);;All files (*)"));
  if (iso.isEmpty()) {
    return;
  }
  performImport(iso);
}

void MainWindow::loadSettings() {
  if (dataRoot_.isEmpty()) {
    applySettings(GameSettings{});
    return;
  }
  applySettings(GameConfig::load(dataRoot_));
}

void MainWindow::applySettings(const GameSettings& value) {
  selectValue(resolutionScale_, value.resolutionScale);
  selectValue(antiAliasing_, value.antiAliasing);
  selectValue(upscaling_, value.upscaling);
  selectValue(anisotropic_, value.anisotropic);
  fullscreen_->setChecked(value.fullscreen);
  vsync_->setChecked(value.vsync);
  frameLimit_->setValue(value.frameLimit);
  nativeMsaa_->setChecked(value.native2xMsaa);
  asyncShaders_->setChecked(value.asyncShaders);
  pipelineThreads_->setValue(value.pipelineThreads);

  forceStereo_->setChecked(value.forceStereo);
  queuedFrames_->setValue(value.audioQueuedFrames);
  xmaWorker_->setChecked(value.xmaWorker);
  frontOnly_->setChecked(value.frontOnly);
  outputGain_->setValue(qRound(value.outputGain * 100.0));
  highpassHz_->setValue(value.highpassHz);

  selectValue(language_, value.languageId);
  country_->setValue(value.countryId);
  logging_->setChecked(value.logging);

  selectValue(inputBackend_, value.inputBackend);
  guideButton_->setChecked(value.guideButton);
  mnkMode_->setChecked(value.mnkMode);
  mnkMouse_->setChecked(value.mnkMouse);
  mnkSensitivity_->setValue(qRound(value.mnkSensitivity * 100.0));
  const QStringList binds = {
      value.keybindA, value.keybindB, value.keybindX, value.keybindY,
      value.keybindLeftTrigger, value.keybindRightTrigger,
      value.keybindLeftShoulder, value.keybindRightShoulder,
      value.keybindLStickUp, value.keybindLStickDown, value.keybindLStickLeft,
      value.keybindLStickRight, value.keybindLStickPress,
      value.keybindRStickUp, value.keybindRStickDown, value.keybindRStickLeft,
      value.keybindRStickRight, value.keybindRStickPress,
      value.keybindDPadUp, value.keybindDPadDown, value.keybindDPadLeft,
      value.keybindDPadRight, value.keybindBack, value.keybindStart,
      value.keybindGuide,
  };
  for (int i = 0; i < binds.size() && i < keybindButtons_.size(); ++i) {
    if (auto* button =
            qobject_cast<KeybindButton*>(keybindButtons_.at(i))) {
      button->setBinding(binds.at(i));
    }
  }
}

GameSettings MainWindow::collectSettings() const {
  GameSettings value;
  value.resolutionScale = resolutionScale_->currentData().toInt();
  value.antiAliasing = antiAliasing_->currentData().toString();
  value.upscaling = upscaling_->currentData().toString();
  value.anisotropic = anisotropic_->currentData().toInt();
  value.fullscreen = fullscreen_->isChecked();
  value.vsync = vsync_->isChecked();
  value.frameLimit = frameLimit_->value();
  value.native2xMsaa = nativeMsaa_->isChecked();
  value.asyncShaders = asyncShaders_->isChecked();
  value.pipelineThreads = pipelineThreads_->value();
  value.forceStereo = forceStereo_->isChecked();
  value.audioQueuedFrames = queuedFrames_->value();
  value.xmaWorker = xmaWorker_->isChecked();
  value.frontOnly = frontOnly_->isChecked();
  value.outputGain = outputGain_->value() / 100.0;
  value.highpassHz = highpassHz_->value();
  value.languageId = language_->currentData().toInt();
  value.countryId = country_->value();
  value.logging = logging_->isChecked();
  value.inputBackend = inputBackend_->currentData().toString();
  value.guideButton = guideButton_->isChecked();
  value.mnkMode = mnkMode_->isChecked();
  value.mnkMouse = mnkMouse_->isChecked();
  value.mnkSensitivity = mnkSensitivity_->value() / 100.0;
  const QStringList bindTexts = {
      keybindButtons_.value(0)->binding(),  keybindButtons_.value(1)->binding(),
      keybindButtons_.value(2)->binding(),  keybindButtons_.value(3)->binding(),
      keybindButtons_.value(4)->binding(),  keybindButtons_.value(5)->binding(),
      keybindButtons_.value(6)->binding(),  keybindButtons_.value(7)->binding(),
      keybindButtons_.value(8)->binding(),  keybindButtons_.value(9)->binding(),
      keybindButtons_.value(10)->binding(), keybindButtons_.value(11)->binding(),
      keybindButtons_.value(12)->binding(), keybindButtons_.value(13)->binding(),
      keybindButtons_.value(14)->binding(), keybindButtons_.value(15)->binding(),
      keybindButtons_.value(16)->binding(), keybindButtons_.value(17)->binding(),
      keybindButtons_.value(18)->binding(), keybindButtons_.value(19)->binding(),
      keybindButtons_.value(20)->binding(), keybindButtons_.value(21)->binding(),
      keybindButtons_.value(22)->binding(), keybindButtons_.value(23)->binding(),
      keybindButtons_.value(24)->binding(),
  };
  value.keybindA = bindTexts.value(0);
  value.keybindB = bindTexts.value(1);
  value.keybindX = bindTexts.value(2);
  value.keybindY = bindTexts.value(3);
  value.keybindLeftTrigger = bindTexts.value(4);
  value.keybindRightTrigger = bindTexts.value(5);
  value.keybindLeftShoulder = bindTexts.value(6);
  value.keybindRightShoulder = bindTexts.value(7);
  value.keybindLStickUp = bindTexts.value(8);
  value.keybindLStickDown = bindTexts.value(9);
  value.keybindLStickLeft = bindTexts.value(10);
  value.keybindLStickRight = bindTexts.value(11);
  value.keybindLStickPress = bindTexts.value(12);
  value.keybindRStickUp = bindTexts.value(13);
  value.keybindRStickDown = bindTexts.value(14);
  value.keybindRStickLeft = bindTexts.value(15);
  value.keybindRStickRight = bindTexts.value(16);
  value.keybindRStickPress = bindTexts.value(17);
  value.keybindDPadUp = bindTexts.value(18);
  value.keybindDPadDown = bindTexts.value(19);
  value.keybindDPadLeft = bindTexts.value(20);
  value.keybindDPadRight = bindTexts.value(21);
  value.keybindBack = bindTexts.value(22);
  value.keybindStart = bindTexts.value(23);
  value.keybindGuide = bindTexts.value(24);
  return value;
}

void MainWindow::saveSettings() {
  if (dataRoot_.isEmpty()) {
    QMessageBox::warning(this, tr("No folder selected"),
                         tr("Select the save data folder first."));
    return;
  }
  QString error;
  if (!GameConfig::save(dataRoot_, collectSettings(), &error)) {
    QMessageBox::critical(this, tr("Error"), error);
    return;
  }
  QMessageBox::information(this, tr("Settings"),
                           tr("Settings saved."));
}

void MainWindow::resetSettings() { applySettings(GameSettings{}); }

QString MainWindow::gameExecutable() const {
  const QDir app(QCoreApplication::applicationDirPath());
  const QStringList candidates{
      app.filePath(QStringLiteral("condemned2recomp")),
      app.filePath(QStringLiteral("../bin/condemned2recomp")),
      app.filePath(QStringLiteral("../condemned2recomp")),
  };
  for (const QString& candidate : candidates) {
    const QFileInfo file(candidate);
    if (file.isFile() && file.isExecutable()) {
      return file.absoluteFilePath();
    }
  }
  return {};
}

void MainWindow::play() {
  if (dataRoot_.isEmpty() || gameRoot_.isEmpty()) {
    QMessageBox::warning(this, tr("Incomplete setup"),
                         tr("Select the save data folder and the game "
                            "folder."));
    return;
  }
  if (!ImportService::hasValidGame(gameRoot_)) {
    QMessageBox::warning(this, tr("Game not found"),
                         tr("Import the ISO first, or select a folder that "
                            "already contains default.xex."));
    return;
  }
  QString error;
  const GameSettings settings = collectSettings();
  if (!GameConfig::save(dataRoot_, settings, &error)) {
    QMessageBox::critical(this, tr("Error"), error);
    return;
  }

  const QString executable = gameExecutable();
  if (executable.isEmpty()) {
    QMessageBox::critical(this, tr("Executable not found"),
                          tr("condemned2recomp was not found next to the launcher."));
    return;
  }

  const QString logsDir = QDir(dataRoot_).filePath(QStringLiteral("logs"));
  QDir().mkpath(logsDir);

  // Launch the game as a managed child process (not detached) so its
  // console output is captured to the logs folder. A silent instant-death
  // is the most common "nothing happens" failure mode.
  if (gameProcess_ != nullptr) {
    gameProcess_->deleteLater();
    gameProcess_ = nullptr;
  }
  gameProcess_ = new QProcess(this);
  gameProcess_->setProgram(executable);
  gameProcess_->setArguments(GameConfig::commandLine(dataRoot_, gameRoot_, settings));
  gameProcess_->setWorkingDirectory(dataRoot_);
  QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
  const QDir app(QCoreApplication::applicationDirPath());
  QStringList libraryPaths{app.absolutePath(),
                           app.absoluteFilePath(QStringLiteral("../lib"))};
  if (environment.contains(QStringLiteral("LD_LIBRARY_PATH"))) {
    libraryPaths << environment.value(QStringLiteral("LD_LIBRARY_PATH"));
  }
  environment.insert(QStringLiteral("LD_LIBRARY_PATH"),
                     libraryPaths.join(QLatin1Char(':')));
  environment.insert(QStringLiteral("DISABLE_LSFG"), QStringLiteral("1"));
  environment.insert(QStringLiteral("XDG_CACHE_HOME"),
                     QDir(dataRoot_).filePath(QStringLiteral("cache")));
  gameProcess_->setProcessEnvironment(environment);
  const QString stdoutLog =
      QDir(logsDir).filePath(QStringLiteral("game-stdout.log"));
  const QString stderrLog =
      QDir(logsDir).filePath(QStringLiteral("game-stderr.log"));
  gameProcess_->setStandardOutputFile(stdoutLog, QIODevice::Truncate);
  gameProcess_->setStandardErrorFile(stderrLog, QIODevice::Truncate);

  // Record the exact command line for diagnostics.
  {
    QFile cmdLog(QDir(logsDir).filePath(QStringLiteral("last-command.txt")));
    if (cmdLog.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream out(&cmdLog);
      out << executable;
      for (const QString& arg : gameProcess_->arguments()) {
        out << ' ' << arg;
      }
      out << '\n';
    }
  }

  connect(gameProcess_,
          QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
          this, &MainWindow::onGameFinished);
  connect(gameProcess_, &QProcess::errorOccurred,
          this, &MainWindow::onGameError);

  statusLabel_->setText(tr("Starting game…"));
  gameStartMsecs_ = QDateTime::currentMSecsSinceEpoch();
  gameProcess_->start();
  if (!gameProcess_->waitForStarted(10000)) {
    QMessageBox::critical(
        this, tr("Could not start"),
        tr("The game did not start:\n%1\n\nSee %2")
            .arg(gameProcess_->errorString(), stderrLog));
    gameProcess_->deleteLater();
    gameProcess_ = nullptr;
    updateStatus();
  }
}

void MainWindow::onGameFinished(int exitCode, QProcess::ExitStatus exitStatus) {
  const qint64 lifetime = QDateTime::currentMSecsSinceEpoch() - gameStartMsecs_;
  updateStatus();
  // A game that dies within ~15s almost certainly failed to initialize.
  // Surface that instead of silently returning to the launcher.
  if (lifetime < 15000 && gameProcess_ != nullptr) {
    const QString logsDir = QDir(dataRoot_).filePath(QStringLiteral("logs"));
    const QString stderrLog =
        QDir(logsDir).filePath(QStringLiteral("game-stderr.log"));
    QString detail;
    QFile logFile(stderrLog);
    if (logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      const QStringList lines =
          QString::fromUtf8(logFile.readAll()).split('\n');
      // Show the last few non-empty lines.
      QStringList tail;
      for (int i = lines.size() - 1; i >= 0 && tail.size() < 8; --i) {
        if (!lines[i].trimmed().isEmpty()) tail.prepend(lines[i].trimmed());
      }
      detail = tail.join('\n');
    }
    QMessageBox::warning(
        this, tr("The game exited unexpectedly"),
        tr("The game closed after %1 ms (exit code %2, %3).\n\n%4\n\nFull output:\n%5")
            .arg(lifetime)
            .arg(exitCode)
            .arg(exitStatus == QProcess::CrashExit ? tr("crashed")
                                                   : tr("exited normally"))
            .arg(detail.isEmpty() ? tr("(no output captured)") : detail)
            .arg(stderrLog));
  }
  if (gameProcess_ != nullptr) {
    gameProcess_->deleteLater();
    gameProcess_ = nullptr;
  }
}

void MainWindow::onGameError(QProcess::ProcessError error) {
  if (error == QProcess::FailedToStart && gameProcess_ != nullptr) {
    QMessageBox::critical(
        this, tr("Could not start"),
        tr("Failed to launch the game:\n%1").arg(gameProcess_->errorString()));
  }
}

void MainWindow::openLogs() {
  if (dataRoot_.isEmpty()) {
    return;
  }
  const QString path = QDir(dataRoot_).filePath(QStringLiteral("logs"));
  QDir().mkpath(path);
  // System tools (xdg-open -> kde-open on KDE) must resolve the *system*
  // Qt, not the one bundled in the AppImage. Launch with LD_LIBRARY_PATH
  // stripped so the loader falls back to the default system paths.
  QProcess opener;
  opener.setProgram(QStringLiteral("xdg-open"));
  opener.setArguments({QDir::toNativeSeparators(path)});
  QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
  environment.remove(QStringLiteral("LD_LIBRARY_PATH"));
  opener.setProcessEnvironment(environment);
  if (!opener.startDetached()) {
    QMessageBox::warning(
        this, tr("Could not open logs"),
        tr("Could not launch the file manager:\n%1").arg(opener.errorString()));
  }
}

void MainWindow::updateStatus() {
  const bool valid = ImportService::hasValidGame(gameRoot_);
  playButton_->setEnabled(valid);
  statusLabel_->setText(
      valid ? tr("Game files detected. Ready to play.")
            : tr("default.xex was not found in the game folder."));
  dataRootLabel_->setText(
      dataRoot_.isEmpty()
          ? tr("No save data folder selected.")
          : tr("Save data: %1").arg(dataRoot_));
  gameRootLabel_->setText(
      gameRoot_.isEmpty()
          ? tr("No game folder selected.")
          : tr("Game: %1").arg(gameRoot_));
  importIsoButton_->setEnabled(!gameRoot_.isEmpty());
}

}  
