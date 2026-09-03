QT += core gui network widgets

TARGET = qmtn
TEMPLATE = app
#CONFIG +=c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

VERSION_RELEASE=0.5
DEFINES += VERSION_RELEASE=\\\"$$VERSION_RELEASE\\\"

use_git_version {
    DEFINES += VERSION_FROM_GIT_TAG=\\\"$$system(git describe --tags --abbrev=4 || echo $$VERSION_RELEASE)\\\"
} else {
    DEFINES += VERSION_FROM_GIT_TAG=\\\"$$VERSION_RELEASE\\\"
}

use_webengine {
    QT += webenginewidgets
}

target.path = /usr/bin
INSTALLS += target

#TODO use macros: VERSION, QMAKE_TARGET_COMPANY, QMAKE_TARGET_DESCRIPTION, QMAKE_TARGET_COPYRIGHT, QMAKE_TARGET_PRODUCT, RC_LANG, RC_CODEPAGE

ikona.path = /usr/share/pixmaps
ikona.files = icons/qmtn.png

desktop.path = /usr/share/applications
desktop.files = assets/qmtn.desktop
unix: INSTALLS += ikona desktop

INCLUDEPATH += uploaders

SOURCES += main.cpp \
    mainwindow.cpp \
    mtnworker.cpp \
    mtnjob.cpp \
    imageitemview.cpp \
    settingsdialog.cpp \
    fileselector.cpp \
    iconprovider.cpp \
    settingsdata.cpp \
    profilemodel.cpp

HEADERS  += mainwindow.h \
    plugins/interfaces.h \
    mtnworker.h \
    mtnjob.h \
    imageitemview.h \
    settingsdialog.h \
    settingsdata.h \
    fileselector.h \
    iconprovider.h \
    profilemodel.h

FORMS    += mainwindow.ui \
    settingsdialog.ui

RESOURCES += images.qrc

ICON = qmtn.ico
RC_ICONS = ../icons/qmtn.ico

OTHER_FILES += \
    ../README.md \
    ../pkgbuild/PKGBUILD

DISTFILES += \
    ../buildrpm/qmtn.spec \
    ../assets/qmtn_create_setup.iss \
    ../README.md \
    ../debian/changelog

TRANSLATIONS = ../lang/sk.ts \
               ../lang/zh_CN.ts
