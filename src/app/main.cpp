#include <QApplication>

#include <spdlog/spdlog.h>

#include "gui/MainWindow.h"
#include "kameraa/Version.h"

int main(int argc, char** argv) {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    spdlog::info("{} {} starting", kameraa::kAppName, kameraa::kVersion);

    QApplication app(argc, argv);
    QApplication::setApplicationName(kameraa::kAppName);
    QApplication::setApplicationVersion(kameraa::kVersion);
    QApplication::setOrganizationName("Kameraa");

    kameraa::MainWindow window;
    window.resize(980, 620);
    window.show();

    const int rc = app.exec();
    spdlog::info("{} exiting ({})", kameraa::kAppName, rc);
    return rc;
}
