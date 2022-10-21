isEmpty(BUILDPATH) {
  error("BUILDPATH must be set")
}

isEmpty(PLUGIN_NAME) {
  error("PLUGIN_NAME must be set")
}

BUILDPATH = $$absolute_path($$BUILDPATH)
PROJPATH = $$absolute_path($$BUILDPATH/..)
S2WPATH = $$absolute_path($$BUILDPATH/../seq2wav)

TEMPLATE = app
TARGET = ../$${PLUGIN_NAME}_gui
OBJECTS_DIR = $$BUILDPATH/gui
MOC_DIR = $$BUILDPATH/gui
RCC_DIR = $$BUILDPATH/gui
CONFIG += debug
QT = core gui widgets multimedia
QMAKE_CXXFLAGS += -std=c++17 -Wno-multichar
DEFINES -= QT_DEPRECATED_WARNINGS
DEFINES += QT_NO_DEPRECATED_WARNINGS
INCLUDEPATH += $$S2WPATH/include
LIBS += $$S2WPATH/build/libseq2wav.a

HEADERS += mainwindow.h
SOURCES += mainwindow.cpp

SOURCES += main.cpp ../plugins/s2wplugin.cpp

SOURCES += $$files($$PROJPATH/src/*.cpp)
SOURCES -= $$PROJPATH/src/main.cpp
for(FN, $$list($$files($$PROJPATH/src/*))) {
  SUB_FILES = $$files($$FN/*.cpp)
  !isEmpty(SUB_FILES) {
    SOURCES += $$SUB_FILES
  }
}
