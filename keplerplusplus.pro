#######################################################
# project file modified!!! Do not run qmake -project!!!!
# if you create new classes, list them hier in the file
# and run qmake afterwards to generate a new Makefile
#######################################################

TEMPLATE = app
DESTDIR=bin
OBJECTS_DIR=lib
MOC_DIR=lib
TARGET = keplerplusplus
INCLUDEPATH += include
INCLUDEPATH += include/simulation

# You can make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# Please consult the documentation of the deprecated API in order to know
# how to port your code away from it.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
SOURCES += $$files(src/*.cpp, true)
HEADERS += $$files(include/*.hpp, true)
HEADERS += $$files(include/*.h, true)

QT += widgets

