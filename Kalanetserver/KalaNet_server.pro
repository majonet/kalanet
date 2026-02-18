QT       += core network

CONFIG += c++17 console
CONFIG -= qt_binpath

TARGET = KalaNetServer
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    src/main_server.cpp \
    src/Product.cpp \
    src/User.cpp \
    src/DataManager.cpp \
    src/Server.cpp

HEADERS += \
    include/Product.h \
    include/User.h \
    include/DataManager.h \
    include/Server.h

INCLUDEPATH += include

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
