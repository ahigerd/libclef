#ifndef CLEF_MAINWINDOW_H
#define CLEF_MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>
#include <QAtomicInteger>
class ClefPluginBase;
class SynthContext;
class TagView;
class PlayerControls;

class MainWindow : public QMainWindow
{
Q_OBJECT
public:
  MainWindow(ClefPluginBase* plugin);

  void openFile(const QString& path, bool autoPlay = false);

protected:
  virtual QWidget* createPluginWidget(QWidget* parent);
  virtual void populateFileMenu(QMenu* fileMenu);
  virtual void populateHelpMenu(QMenu* helpMenu);
  void lockWhileWorking(QWidget* widget);
  void lockWhileWorking(QAction* action);

signals:
  void contextUpdated(SynthContext* ctx);
  void loadError(const QString& filename, const QString& msg);
  void exportStarted();

private slots:
  void initUI();
  void createPluginWidget();
  void openFile();
  void exportFile();
  void about();
  void unlockWork();
  void onLoadError(const QString& filename, const QString& msg);
  void openSubsong(const QString& filename);

private:
  bool lockWork();
  void openFile(const QString& path, bool doAcquire, bool autoPlay);

protected:
  ClefPluginBase* m_plugin;
  SynthContext* ctx;
  PlayerControls* controls;

private:
  TagView* tagView;
  QAtomicInteger<bool> busy;
  QString currentFile;
  QString queuedLoad;
  bool m_autoPlay;

  QList<QPointer<QWidget>> lockWidgets;
  QList<QPointer<QAction>> lockActions;
  QPointer<QWidget> pluginWidget;
};

#endif
