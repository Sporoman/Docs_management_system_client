QT += core
QT += gui
QT += network
QT += printsupport
QT += charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    commands.cpp \
    docforshowinfo.cpp \
    docs.cpp \
    docsedit.cpp \
    login.cpp \
    main.cpp \
    mainwindow.cpp \
    profilecheck.cpp \
    profilecreate.cpp \
    profileedit.cpp \
    searcher.cpp \
    socket_job.cpp \
    statistics.cpp \
    upload.cpp \
    userprofile.cpp

HEADERS += \
    commands.h \
    docforshowinfo.h \
    docs.h \
    docsedit.h \
    login.h \
    mainwindow.h \
    profilecheck.h \
    profilecreate.h \
    profileedit.h \
    searcher.h \
    socket_job.h \
    statistics.h \
    upload.h \
    userprofile.h

FORMS += \
    docforshowinfo.ui \
    docs.ui \
    docsedit.ui \
    login.ui \
    mainwindow.ui \
    profilecheck.ui \
    profilecreate.ui \
    profileedit.ui \
    searcher.ui \
    statistics.ui \
    upload.ui \
    userprofile.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
