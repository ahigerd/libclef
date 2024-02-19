isEmpty(CLEF_LDFLAGS) {
  error("CLEF_LDFLAGS must be set")
}

isEmpty(BUILDPATH) {
  error("BUILDPATH must be set")
}

isEmpty(PLUGIN_NAME) {
  error("PLUGIN_NAME must be set")
}

PROJPATH = $$absolute_path($$BUILDPATH/..)
CLEF_PATH = $$absolute_path($$BUILDPATH/../libclef)
CLEF_BUILDPATH = $$absolute_path($$CLEF_PATH/$$relative_path($$BUILDPATH,$$PROJPATH))

TEMPLATE = app

isEmpty(BUILD_DEBUG) {
  TARGET = ../$${PLUGIN_NAME}_gui
  OBJECTS_DIR = $$BUILDPATH/gui
  MOC_DIR = $$BUILDPATH/gui
  RCC_DIR = $$BUILDPATH/gui
  CONFIG -= debug debug_and_release
  CONFIG += release
  LIBS += -L$$BUILDPATH -l$$PLUGIN_NAME
  PRE_TARGETDEPS += $$BUILDPATH/lib$${PLUGIN_NAME}.a $$CLEF_BUILDPATH/libclef.a
} else {
  TARGET = ../$${PLUGIN_NAME}_gui_d
  OBJECTS_DIR = $$BUILDPATH/gui_d
  MOC_DIR = $$BUILDPATH/gui_d
  RCC_DIR = $$BUILDPATH/gui_d
  CONFIG -= release debug_and_release
  CONFIG += debug
  LIBS += -L$$BUILDPATH -l$${PLUGIN_NAME}_d
  PRE_TARGETDEPS += $$BUILDPATH/lib$${PLUGIN_NAME}_d.a $$CLEF_BUILDPATH/libclef_d.a
}
QT = core gui widgets multimedia
CONFIG += c++17
QMAKE_CXXFLAGS += -std=c++17 -Wno-multichar
DEFINES -= QT_DEPRECATED_WARNINGS
DEFINES += QT_NO_DEPRECATED_WARNINGS
INCLUDEPATH += $$CLEF_PATH/src $$CLEF_PATH/gui $$PROJPATH/src
LIBS += $$CLEF_LDFLAGS

CLEF_CLASSES = mainwindow tagview playercontrols vumeter
for (CLS,CLEF_CLASSES) {
  exists($$CLEF_PATH/gui/$${CLS}.h):HEADERS += $$CLEF_PATH/gui/$${CLS}.h
  exists($$CLEF_PATH/gui/$${CLS}.cpp):SOURCES += $$CLEF_PATH/gui/$${CLS}.cpp
}
