QT       += core gui widgets network

CONFIG += c++17 console

TARGET = KalaNetClient
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    src/main.cpp \
    src/Product.cpp \
    src/User.cpp \
    src/LoginDialog.cpp \
    src/MainWindow.cpp \
    src/DataManager.cpp \
    src/NetworkManager.cpp

HEADERS += \
    include/Product.h \
    include/User.h \
    include/LoginDialog.h \
    include/MainWindow.h \
    include/DataManager.h \
    include/NetworkManager.h

INCLUDEPATH += include

# Remove DataManager from client build - it's now server-only
# DataManager.cpp is NOT included in client build

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
