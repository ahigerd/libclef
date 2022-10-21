#include <QApplication>
#include "mainwindow.h"
#include "s2wcontext.h"
#include "synth/synthcontext.h"
#include "plugin/baseplugin.h"

int main(int argc, char** argv)
{
  S2WContext ctx;
  S2WPluginBase* plugin = S2W::makePlugin(&ctx);

  QCoreApplication::setApplicationName(QString::fromStdString(plugin->pluginName()));
  QCoreApplication::setApplicationVersion(QString::fromStdString(plugin->version()));
  QCoreApplication::setOrganizationName("seq2wav");
  QCoreApplication::setOrganizationDomain("seq2wav" + QString::fromStdString(plugin->pluginShortName()));
  QApplication app(argc, argv);

  MainWindow mw(plugin);
  mw.show();

  return app.exec();
}
