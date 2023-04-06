isEmpty(S2W_LDFLAGS) {
  error("S2W_LDFLAGS must be set")
}

isEmpty(BUILDPATH) {
  error("BUILDPATH must be set")
}

isEmpty(PLUGIN_NAME) {
  error("PLUGIN_NAME must be set")
}

PROJPATH = $$absolute_path($$BUILDPATH/..)
S2WPATH = $$absolute_path($$BUILDPATH/../seq2wav)
S2WBUILDPATH = $$absolute_path($$S2WPATH/$$relative_path($$BUILDPATH,$$PROJPATH))

TEMPLATE = app

isEmpty(BUILD_DEBUG) {
  TARGET = ../$${PLUGIN_NAME}_gui
  OBJECTS_DIR = $$BUILDPATH/gui
  MOC_DIR = $$BUILDPATH/gui
  RCC_DIR = $$BUILDPATH/gui
  CONFIG -= debug debug_and_release
  CONFIG += release
  LIBS += -L$$BUILDPATH -l$$PLUGIN_NAME
  PRE_TARGETDEPS += $$BUILDPATH/lib$${PLUGIN_NAME}.a $$S2WBUILDPATH/libseq2wav.a
} else {
  TARGET = ../$${PLUGIN_NAME}_gui_d
  OBJECTS_DIR = $$BUILDPATH/gui_d
  MOC_DIR = $$BUILDPATH/gui_d
  RCC_DIR = $$BUILDPATH/gui_d
  CONFIG -= release debug_and_release
  CONFIG += debug
  LIBS += -L$$BUILDPATH -l$${PLUGIN_NAME}_d
  PRE_TARGETDEPS += $$BUILDPATH/lib$${PLUGIN_NAME}_d.a $$S2WBUILDPATH/libseq2wav_d.a
}
QT = core gui widgets multimedia
CONFIG += c++17
QMAKE_CXXFLAGS += -std=c++17 -Wno-multichar
DEFINES -= QT_DEPRECATED_WARNINGS
DEFINES += QT_NO_DEPRECATED_WARNINGS
INCLUDEPATH += $$S2WPATH/include $$S2WPATH/gui $$PROJPATH/src
LIBS += $$S2W_LDFLAGS

S2W_CLASSES = mainwindow tagview playercontrols vumeter
for (CLS,S2W_CLASSES) {
  exists($$S2WPATH/gui/$${CLS}.h):HEADERS += $$S2WPATH/gui/$${CLS}.h
  exists($$S2WPATH/gui/$${CLS}.cpp):SOURCES += $$S2WPATH/gui/$${CLS}.cpp
}
