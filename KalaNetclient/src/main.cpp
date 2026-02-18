#include <QApplication>
#include <QEventLoop>
#include <QMessageBox>
#include "LoginDialog.h"
#include "MainWindow.h"
#include "NetworkManager.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName("KalaNet");
    app.setOrganizationName("IUT");

    NetworkManager* net = NetworkManager::instance();

    // Try to connect to the server; retry on failure
    while (true) {
        if (net->connectToServer("127.0.0.1", 12345)) {
            break;
        }

        QMessageBox::StandardButton reply = QMessageBox::critical(
            nullptr,
            "Connection Error",
            "Cannot connect to the server.\n"
            "Make sure the server is running on port 12345.\n\n"
            "Do you want to retry?",
            QMessageBox::Retry | QMessageBox::Abort
            );

        if (reply == QMessageBox::Abort) {
            NetworkManager::destroy();
            return 1;
        }
    }

    // Main application loop – returns to login after logout
    while (true) {
        LoginDialog loginDialog;

        if (loginDialog.exec() != QDialog::Accepted) {
            break;  // User closed login dialog - exit completely
        }

        User* user = loginDialog.getAuthenticatedUser();
        if (!user) {
            QMessageBox::critical(nullptr, "Error", "Authentication failed");
            continue;  // Back to login
        }

        // Create MainWindow on heap (not stack) so deleteLater() works properly
        MainWindow* mainWindow = new MainWindow(user);
        bool logoutClicked = false;

        // Connect logout signal to restart flag
        QObject::connect(mainWindow, &MainWindow::logoutRequested, [&logoutClicked]() {
            logoutClicked = true;
        });

        mainWindow->show();

        // Run event loop until the main window is destroyed
        QEventLoop loop;
        QObject::connect(mainWindow, &MainWindow::destroyed, &loop, &QEventLoop::quit);
        QObject::connect(net, &NetworkManager::disconnected, &loop, &QEventLoop::quit);
        loop.exec();

        // Clean up - delete if not already deleted by deleteLater()
        if (mainWindow) {
            delete mainWindow;
        }

        // If logout was not clicked (window was closed by X button), exit app
        if (!logoutClicked) {
            break;
        }
        // If logoutClicked is true, loop continues to show login again
    }

    NetworkManager::destroy();
    return 0;
}
