#include <QApplication>
#include "LoginDialog.h"
#include "MainWindow.h"
#include "DataManager.h"
#include <QMessageBox>
#include <QDebug>
#include <QDir>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName("KalaNet");
    app.setOrganizationName("IUT");

    qDebug() << "Starting KalaNet...";
    qDebug() << "Working directory:" << QDir::currentPath();

    // Initialize DataManager (loads existing data)
    DataManager* dm = DataManager::getInstance();

    qDebug() << "DataManager initialized";
    qDebug() << "Total users:" << dm->getTotalUsers();
    qDebug() << "Total products:" << dm->getTotalProducts();

    bool running = true;

    while (running) {
        // Show login dialog
        LoginDialog loginDialog;

        if (loginDialog.exec() != QDialog::Accepted) {
            // User cancelled login
            running = false;
            break;
        }

        User* user = loginDialog.getAuthenticatedUser();
        if (!user) {
            QMessageBox::critical(nullptr, "Error", "Authentication failed");
            continue;
        }

        qDebug() << "User logged in:" << user->getUsername() 
                 << "Type:" << user->getUserTypeString();

        // Show main window
        MainWindow mainWindow(user);
        mainWindow.show();

        // Run the application
        int result = app.exec();

        // Check if we should restart (logout) or quit
        if (result == 0) {
            // Normal close
            running = false;
        } else {
            // Logout - continue loop to show login again
            qDebug() << "User logged out, returning to login...";
            continue;
        }
    }

    // Cleanup
    qDebug() << "Saving data and exiting...";
    DataManager::destroyInstance();

    return 0;
}
