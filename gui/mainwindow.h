#ifndef S2W_MAINWINDOW_H
#define S2W_MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>
#include <QAtomicInteger>
class S2WPluginBase;
class SynthContext;
class TagView;
class PlayerControls;

class MainWindow : public QMainWindow
{
Q_OBJECT
public:
  MainWindow(S2WPluginBase* plugin);

  void openFile(const QString& path, bool autoPlay = false);

protected:
  virtual QWidget* createPluginWidget(QWidget* parent);
  void lockWhileWorking(QWidget* widget);
  void lockWhileWorking(QAction* action);

signals:
  void contextUpdated(SynthContext* ctx);
  void loadError(const QString& filename, const QString& msg);

private slots:
  void createPluginWidget();
  void openFile();
  void about();
  void unlockWork();
  void onLoadError(const QString& filename, const QString& msg);

private:
  bool lockWork();
  void openFile(const QString& path, bool doAcquire, bool autoPlay);

protected:
  S2WPluginBase* m_plugin;
  SynthContext* ctx;
  PlayerControls* controls;

private:
  TagView* tagView;
  QAtomicInteger<bool> busy;
  bool m_autoPlay;

  QList<QPointer<QWidget>> lockWidgets;
  QList<QPointer<QAction>> lockActions;
  QPointer<QWidget> pluginWidget;
};

#endif
