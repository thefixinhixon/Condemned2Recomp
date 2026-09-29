#pragma once

#include <QSettings>
#include <QString>
#include <QStringList>

namespace condemned2::launcher {

struct GameSettings {
  int resolutionScale = 1;
  QString antiAliasing = QStringLiteral("none");
  int anisotropic = 16;
  bool fullscreen = true;
  bool vsync = true;
  int frameLimit = 60;
  QString upscaling = QStringLiteral("bilinear");
  bool native2xMsaa = false;
  bool asyncShaders = true;
  int pipelineThreads = -1;

  bool forceStereo = true;
  int audioQueuedFrames = 16;
  bool xmaWorker = false;
  bool frontOnly = true;
  double outputGain = 0.5;
  int highpassHz = 100;

  int languageId = 1;
  int countryId = 103;
  bool logging = true;

  // Input / controls (ReXGlue mnk driver + SDL backend).
  QString inputBackend = QStringLiteral("sdl");
  bool guideButton = false;
  bool mnkMode = false;
  bool mnkMouse = false;
  double mnkSensitivity = 1.0;
  QString keybindA = QStringLiteral("Semicolon,Space");
  QString keybindB = QStringLiteral("Quote,Backspace");
  QString keybindX = QStringLiteral("L");
  QString keybindY = QStringLiteral("P");
  QString keybindLeftTrigger = QStringLiteral("Q,I");
  QString keybindRightTrigger = QStringLiteral("E,O");
  QString keybindLeftShoulder = QStringLiteral("1");
  QString keybindRightShoulder = QStringLiteral("3");
  QString keybindLStickUp = QStringLiteral("W");
  QString keybindLStickDown = QStringLiteral("S");
  QString keybindLStickLeft = QStringLiteral("A");
  QString keybindLStickRight = QStringLiteral("D");
  QString keybindLStickPress = QStringLiteral("F");
  QString keybindRStickUp = QStringLiteral("Up");
  QString keybindRStickDown = QStringLiteral("Down");
  QString keybindRStickLeft = QStringLiteral("Left");
  QString keybindRStickRight = QStringLiteral("Right");
  QString keybindRStickPress = QStringLiteral("K");
  QString keybindDPadUp = QStringLiteral("Shift+Up");
  QString keybindDPadDown = QStringLiteral("Shift+Down");
  QString keybindDPadLeft = QStringLiteral("Shift+Left");
  QString keybindDPadRight = QStringLiteral("Shift+Right");
  QString keybindBack = QStringLiteral("Z,Tab");
  QString keybindStart = QStringLiteral("X,Return");
  QString keybindGuide = QStringLiteral("");
};

class GameConfig {
 public:
  static QString globalConfigPath();
  static QString dataRootConfigPath(const QString& dataRoot);
  static QString selectedDataRoot();
  static void setSelectedDataRoot(const QString& root);
  static QString selectedGameRoot();
  static void setSelectedGameRoot(const QString& root);

  static GameSettings load(const QString& dataRoot);
  static bool save(const QString& dataRoot, const GameSettings& settings,
                   QString* error = nullptr);
  static QStringList commandLine(const QString& dataRoot, const QString& gameRoot,
                                 const GameSettings& settings);

 private:
  static GameSettings read(QSettings& settings);
  static void write(QSettings& out, const GameSettings& settings);
};

}  
