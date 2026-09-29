#include "game_config.h"

#include <QDir>
#include <QStandardPaths>

namespace condemned2::launcher {
namespace {

QString booleanArgument(const char* name, bool value) {
  return QStringLiteral("--%1=%2")
      .arg(QString::fromLatin1(name), value ? QStringLiteral("true")
                                            : QStringLiteral("false"));
}

QString valueArgument(const char* name, const QString& value) {
  return QStringLiteral("--%1=%2").arg(QString::fromLatin1(name), value);
}

}  

QString GameConfig::globalConfigPath() {
  const QString directory =
      QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
  QDir().mkpath(directory);
  return QDir(directory).filePath(QStringLiteral("launcher.ini"));
}

QString GameConfig::dataRootConfigPath(const QString& dataRoot) {
  return QDir(dataRoot).filePath(QStringLiteral("config/launcher.ini"));
}

QString GameConfig::selectedDataRoot() {
  QSettings settings(globalConfigPath(), QSettings::IniFormat);
  return QDir::cleanPath(
      settings.value(QStringLiteral("paths/data_root")).toString());
}

void GameConfig::setSelectedDataRoot(const QString& root) {
  QSettings settings(globalConfigPath(), QSettings::IniFormat);
  settings.setValue(QStringLiteral("paths/data_root"),
                    QDir::cleanPath(QDir(root).absolutePath()));
  settings.sync();
}

QString GameConfig::selectedGameRoot() {
  QSettings settings(globalConfigPath(), QSettings::IniFormat);
  return QDir::cleanPath(
      settings.value(QStringLiteral("paths/game_root")).toString());
}

void GameConfig::setSelectedGameRoot(const QString& root) {
  QSettings settings(globalConfigPath(), QSettings::IniFormat);
  settings.setValue(QStringLiteral("paths/game_root"),
                    QDir::cleanPath(QDir(root).absolutePath()));
  settings.sync();
}

GameSettings GameConfig::read(QSettings& in) {
  GameSettings result;
  result.resolutionScale =
      in.value(QStringLiteral("graphics/resolution_scale"), 1).toInt();
  result.antiAliasing =
      in.value(QStringLiteral("graphics/swap_post_effect"), QStringLiteral("none"))
          .toString();
  result.anisotropic =
      in.value(QStringLiteral("graphics/anisotropic_override"), 16).toInt();
  result.fullscreen =
      in.value(QStringLiteral("graphics/fullscreen"), true).toBool();
  result.vsync = in.value(QStringLiteral("graphics/vsync"), true).toBool();
  result.frameLimit =
      in.value(QStringLiteral("graphics/frame_limit"), 60).toInt();
  result.upscaling =
      in.value(QStringLiteral("graphics/upscaling"), QStringLiteral("bilinear"))
          .toString();
  result.native2xMsaa =
      in.value(QStringLiteral("graphics/native_2x_msaa"), false).toBool();
  result.asyncShaders =
      in.value(QStringLiteral("graphics/async_shader_compilation"), true).toBool();
  result.pipelineThreads =
      in.value(QStringLiteral("graphics/vulkan_pipeline_creation_threads"), -1)
          .toInt();

  result.forceStereo =
      in.value(QStringLiteral("audio/force_stereo"), true).toBool();
  result.audioQueuedFrames =
      in.value(QStringLiteral("audio/max_queued_frames"), 16).toInt();
  result.xmaWorker =
      in.value(QStringLiteral("audio/xma_worker"), false).toBool();
  result.frontOnly =
      in.value(QStringLiteral("audio/front_only"), true).toBool();
  result.outputGain =
      in.value(QStringLiteral("audio/output_gain"), 0.5).toDouble();
  result.highpassHz =
      in.value(QStringLiteral("audio/highpass_hz"), 100).toInt();

  result.languageId =
      in.value(QStringLiteral("locale/language_id"), 1).toInt();
  result.countryId = in.value(QStringLiteral("locale/country_id"), 103).toInt();
  result.logging = in.value(QStringLiteral("general/logging"), true).toBool();

  result.inputBackend =
      in.value(QStringLiteral("input/input_backend"), QStringLiteral("sdl"))
          .toString();
  result.guideButton =
      in.value(QStringLiteral("input/guide_button"), false).toBool();
  result.mnkMode = in.value(QStringLiteral("input/mnk_mode"), false).toBool();
  result.mnkMouse =
      in.value(QStringLiteral("input/mnk_mouse"), false).toBool();
  result.mnkSensitivity =
      in.value(QStringLiteral("input/mnk_sensitivity"), 1.0).toDouble();
  result.keybindA = in.value(QStringLiteral("input/keybind_a"),
                             QStringLiteral("Semicolon,Space")).toString();
  result.keybindB = in.value(QStringLiteral("input/keybind_b"),
                             QStringLiteral("Quote,Backspace")).toString();
  result.keybindX =
      in.value(QStringLiteral("input/keybind_x"), QStringLiteral("L")).toString();
  result.keybindY =
      in.value(QStringLiteral("input/keybind_y"), QStringLiteral("P")).toString();
  result.keybindLeftTrigger = in.value(QStringLiteral("input/keybind_left_trigger"),
                                       QStringLiteral("Q,I")).toString();
  result.keybindRightTrigger = in.value(QStringLiteral("input/keybind_right_trigger"),
                                        QStringLiteral("E,O")).toString();
  result.keybindLeftShoulder = in.value(QStringLiteral("input/keybind_left_shoulder"),
                                       QStringLiteral("1")).toString();
  result.keybindRightShoulder = in.value(QStringLiteral("input/keybind_right_shoulder"),
                                        QStringLiteral("3")).toString();
  result.keybindLStickUp = in.value(QStringLiteral("input/keybind_lstick_up"),
                                   QStringLiteral("W")).toString();
  result.keybindLStickDown = in.value(QStringLiteral("input/keybind_lstick_down"),
                                     QStringLiteral("S")).toString();
  result.keybindLStickLeft = in.value(QStringLiteral("input/keybind_lstick_left"),
                                     QStringLiteral("A")).toString();
  result.keybindLStickRight = in.value(QStringLiteral("input/keybind_lstick_right"),
                                      QStringLiteral("D")).toString();
  result.keybindLStickPress = in.value(QStringLiteral("input/keybind_lstick_press"),
                                      QStringLiteral("F")).toString();
  result.keybindRStickUp = in.value(QStringLiteral("input/keybind_rstick_up"),
                                   QStringLiteral("Up")).toString();
  result.keybindRStickDown = in.value(QStringLiteral("input/keybind_rstick_down"),
                                     QStringLiteral("Down")).toString();
  result.keybindRStickLeft = in.value(QStringLiteral("input/keybind_rstick_left"),
                                     QStringLiteral("Left")).toString();
  result.keybindRStickRight = in.value(QStringLiteral("input/keybind_rstick_right"),
                                      QStringLiteral("Right")).toString();
  result.keybindRStickPress = in.value(QStringLiteral("input/keybind_rstick_press"),
                                      QStringLiteral("K")).toString();
  result.keybindDPadUp = in.value(QStringLiteral("input/keybind_dpad_up"),
                                 QStringLiteral("Shift+Up")).toString();
  result.keybindDPadDown = in.value(QStringLiteral("input/keybind_dpad_down"),
                                   QStringLiteral("Shift+Down")).toString();
  result.keybindDPadLeft = in.value(QStringLiteral("input/keybind_dpad_left"),
                                   QStringLiteral("Shift+Left")).toString();
  result.keybindDPadRight = in.value(QStringLiteral("input/keybind_dpad_right"),
                                    QStringLiteral("Shift+Right")).toString();
  result.keybindBack = in.value(QStringLiteral("input/keybind_back"),
                               QStringLiteral("Z,Tab")).toString();
  result.keybindStart = in.value(QStringLiteral("input/keybind_start"),
                                QStringLiteral("X,Return")).toString();
  result.keybindGuide =
      in.value(QStringLiteral("input/keybind_guide"), QStringLiteral("")).toString();
  return result;
}

void GameConfig::write(QSettings& out, const GameSettings& value) {
  out.setValue(QStringLiteral("graphics/resolution_scale"),
               value.resolutionScale);
  out.setValue(QStringLiteral("graphics/swap_post_effect"),
               value.antiAliasing);
  out.setValue(QStringLiteral("graphics/anisotropic_override"),
               value.anisotropic);
  out.setValue(QStringLiteral("graphics/fullscreen"), value.fullscreen);
  out.setValue(QStringLiteral("graphics/vsync"), value.vsync);
  out.setValue(QStringLiteral("graphics/frame_limit"), value.frameLimit);
  out.setValue(QStringLiteral("graphics/upscaling"), value.upscaling);
  out.setValue(QStringLiteral("graphics/native_2x_msaa"), value.native2xMsaa);
  out.setValue(QStringLiteral("graphics/async_shader_compilation"),
               value.asyncShaders);
  out.setValue(QStringLiteral("graphics/vulkan_pipeline_creation_threads"),
               value.pipelineThreads);

  out.setValue(QStringLiteral("audio/force_stereo"), value.forceStereo);
  out.setValue(QStringLiteral("audio/max_queued_frames"),
               value.audioQueuedFrames);
  out.setValue(QStringLiteral("audio/xma_worker"), value.xmaWorker);
  out.setValue(QStringLiteral("audio/front_only"), value.frontOnly);
  out.setValue(QStringLiteral("audio/output_gain"), value.outputGain);
  out.setValue(QStringLiteral("audio/highpass_hz"), value.highpassHz);

  out.setValue(QStringLiteral("locale/language_id"), value.languageId);
  out.setValue(QStringLiteral("locale/country_id"), value.countryId);
  out.setValue(QStringLiteral("general/logging"), value.logging);

  out.setValue(QStringLiteral("input/input_backend"), value.inputBackend);
  out.setValue(QStringLiteral("input/guide_button"), value.guideButton);
  out.setValue(QStringLiteral("input/mnk_mode"), value.mnkMode);
  out.setValue(QStringLiteral("input/mnk_mouse"), value.mnkMouse);
  out.setValue(QStringLiteral("input/mnk_sensitivity"), value.mnkSensitivity);
  out.setValue(QStringLiteral("input/keybind_a"), value.keybindA);
  out.setValue(QStringLiteral("input/keybind_b"), value.keybindB);
  out.setValue(QStringLiteral("input/keybind_x"), value.keybindX);
  out.setValue(QStringLiteral("input/keybind_y"), value.keybindY);
  out.setValue(QStringLiteral("input/keybind_left_trigger"), value.keybindLeftTrigger);
  out.setValue(QStringLiteral("input/keybind_right_trigger"), value.keybindRightTrigger);
  out.setValue(QStringLiteral("input/keybind_left_shoulder"), value.keybindLeftShoulder);
  out.setValue(QStringLiteral("input/keybind_right_shoulder"), value.keybindRightShoulder);
  out.setValue(QStringLiteral("input/keybind_lstick_up"), value.keybindLStickUp);
  out.setValue(QStringLiteral("input/keybind_lstick_down"), value.keybindLStickDown);
  out.setValue(QStringLiteral("input/keybind_lstick_left"), value.keybindLStickLeft);
  out.setValue(QStringLiteral("input/keybind_lstick_right"), value.keybindLStickRight);
  out.setValue(QStringLiteral("input/keybind_lstick_press"), value.keybindLStickPress);
  out.setValue(QStringLiteral("input/keybind_rstick_up"), value.keybindRStickUp);
  out.setValue(QStringLiteral("input/keybind_rstick_down"), value.keybindRStickDown);
  out.setValue(QStringLiteral("input/keybind_rstick_left"), value.keybindRStickLeft);
  out.setValue(QStringLiteral("input/keybind_rstick_right"), value.keybindRStickRight);
  out.setValue(QStringLiteral("input/keybind_rstick_press"), value.keybindRStickPress);
  out.setValue(QStringLiteral("input/keybind_dpad_up"), value.keybindDPadUp);
  out.setValue(QStringLiteral("input/keybind_dpad_down"), value.keybindDPadDown);
  out.setValue(QStringLiteral("input/keybind_dpad_left"), value.keybindDPadLeft);
  out.setValue(QStringLiteral("input/keybind_dpad_right"), value.keybindDPadRight);
  out.setValue(QStringLiteral("input/keybind_back"), value.keybindBack);
  out.setValue(QStringLiteral("input/keybind_start"), value.keybindStart);
  out.setValue(QStringLiteral("input/keybind_guide"), value.keybindGuide);
}

GameSettings GameConfig::load(const QString& dataRoot) {
  QSettings settings(dataRootConfigPath(dataRoot), QSettings::IniFormat);
  return read(settings);
}

bool GameConfig::save(const QString& dataRoot, const GameSettings& value,
                      QString* error) {
  const QString configDirectory =
      QDir(dataRoot).filePath(QStringLiteral("config"));
  if (!QDir().mkpath(configDirectory)) {
    if (error) {
      *error = QStringLiteral("Could not create %1").arg(configDirectory);
    }
    return false;
  }

  QSettings settings(dataRootConfigPath(dataRoot), QSettings::IniFormat);
  write(settings, value);
  settings.sync();
  if (settings.status() != QSettings::NoError) {
    if (error) {
      *error = QStringLiteral("Could not save settings.");
    }
    return false;
  }
  return true;
}

QStringList GameConfig::commandLine(const QString& dataRoot,
                                    const QString& gameRoot,
                                    const GameSettings& value) {
  QStringList result{
      valueArgument("game_data_root", QDir(gameRoot).absolutePath()),
      valueArgument("user_data_root", QDir(dataRoot).absolutePath()),
      valueArgument("resolution_scale",
                    QString::number(value.resolutionScale)),
      valueArgument("swap_post_effect", value.antiAliasing),
      valueArgument("anisotropic_override",
                    QString::number(value.anisotropic)),
      booleanArgument("fullscreen", value.fullscreen),
      booleanArgument("vsync", value.vsync),
      valueArgument("frame_limit", QString::number(value.frameLimit)),
      valueArgument("present_effect", value.upscaling),
      booleanArgument("native_2x_msaa", value.native2xMsaa),
      booleanArgument("async_shader_compilation", value.asyncShaders),
      valueArgument("vulkan_pipeline_creation_threads",
                    QString::number(value.pipelineThreads)),
#if defined(Q_PROCESSOR_ARM_64)
      // Turnip/ARM64 workaround: force the fast FBO render target path.
      // (On x86_64 the SDK default already selects the same host-render-target
      // path, so no override is needed there.)
      valueArgument("render_target_path_vulkan", QStringLiteral("fbo")),
#endif
      // Sparse shared memory is disabled on all platforms: it black-screens
      // on AMD RX 6600/RADV (and is buggy on mobile drivers), so force the
      // reliable non-sparse shared memory buffer.
      booleanArgument("vulkan_sparse_shared_memory", false),
      booleanArgument("audio_force_stereo", value.forceStereo),
      valueArgument("audio_maxqframes",
                    QString::number(value.audioQueuedFrames)),
      booleanArgument("xma_worker", value.xmaWorker),
      booleanArgument("audio_front_only", value.frontOnly),
      valueArgument("audio_output_gain",
                    QString::number(value.outputGain, 'f', 2)),
      valueArgument("audio_highpass_hz", QString::number(value.highpassHz)),
      valueArgument("user_language", QString::number(value.languageId)),
      valueArgument("user_country", QString::number(value.countryId)),
      valueArgument("input_backend", value.inputBackend),
      booleanArgument("guide_button", value.guideButton),
      booleanArgument("mnk_mode", value.mnkMode),
      booleanArgument("mnk_mouse", value.mnkMouse),
      valueArgument("mnk_sensitivity",
                    QString::number(value.mnkSensitivity, 'f', 2)),
      valueArgument("keybind_a", value.keybindA),
      valueArgument("keybind_b", value.keybindB),
      valueArgument("keybind_x", value.keybindX),
      valueArgument("keybind_y", value.keybindY),
      valueArgument("keybind_left_trigger", value.keybindLeftTrigger),
      valueArgument("keybind_right_trigger", value.keybindRightTrigger),
      valueArgument("keybind_left_shoulder", value.keybindLeftShoulder),
      valueArgument("keybind_right_shoulder", value.keybindRightShoulder),
      valueArgument("keybind_lstick_up", value.keybindLStickUp),
      valueArgument("keybind_lstick_down", value.keybindLStickDown),
      valueArgument("keybind_lstick_left", value.keybindLStickLeft),
      valueArgument("keybind_lstick_right", value.keybindLStickRight),
      valueArgument("keybind_lstick_press", value.keybindLStickPress),
      valueArgument("keybind_rstick_up", value.keybindRStickUp),
      valueArgument("keybind_rstick_down", value.keybindRStickDown),
      valueArgument("keybind_rstick_left", value.keybindRStickLeft),
      valueArgument("keybind_rstick_right", value.keybindRStickRight),
      valueArgument("keybind_rstick_press", value.keybindRStickPress),
      valueArgument("keybind_dpad_up", value.keybindDPadUp),
      valueArgument("keybind_dpad_down", value.keybindDPadDown),
      valueArgument("keybind_dpad_left", value.keybindDPadLeft),
      valueArgument("keybind_dpad_right", value.keybindDPadRight),
      valueArgument("keybind_back", value.keybindBack),
      valueArgument("keybind_start", value.keybindStart),
      valueArgument("keybind_guide", value.keybindGuide),
      valueArgument("log_level",
                    value.logging ? QStringLiteral("info")
                                  : QStringLiteral("off")),
      valueArgument("log_file",
                    QDir(dataRoot).filePath(QStringLiteral("logs/condemned2recomp.log"))),
      valueArgument("storage_root", QDir(dataRoot).absolutePath()),
  };
  return result;
}

}  
