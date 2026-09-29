#include "main_window.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char** argv) {
  QApplication application(argc, argv);
  QCoreApplication::setOrganizationName(QStringLiteral("HellsGateRecomp"));
  QCoreApplication::setApplicationName(QStringLiteral("Condemned2Launcher"));
  QCoreApplication::setApplicationVersion(
      QStringLiteral(CONDEMNED2_LAUNCHER_VERSION));

  condemned2::launcher::MainWindow window;
  window.show();
  return application.exec();
}
