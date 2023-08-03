QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    chatwindow.cpp \
    extendedqlistwidgetitem.cpp \
    friend.cpp \
    friendsstatuses.cpp \
    invitation.cpp \
    main.cpp \
    loginpage.cpp \
    mainwindow.cpp \
    messagescontroller.cpp \
    queries.cpp \
    signupwindow.cpp \
    user.cpp

HEADERS += \
    chatwindow.h \
    extendedqlistwidgetitem.h \
    friend.h \
    friendsstatuses.h \
    invitation.h \
    loginpage.h \
    mainwindow.h \
    messagescontroller.h \
    queries.h \
    signupwindow.h \
    user.h

FORMS += \
    chatwindow.ui \
    loginpage.ui \
    mainwindow.ui \
    signupwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc
