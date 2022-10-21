#ifndef S2W_MAINWINDOW_H
#define S2W_MAINWINDOW_H

#include <QMainWindow>
class S2WPluginBase;
class SynthContext;

class MainWindow : public QMainWindow
{
Q_OBJECT
public:
  MainWindow(S2WPluginBase* plugin);

private slots:
  void openFile();
  void about();

private:
  S2WPluginBase* m_plugin;
  SynthContext* ctx;
};

#endif
