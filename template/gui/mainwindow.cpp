#include "mainwindow.h"
#include "plugin/baseplugin.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>

MainWindow::MainWindow(S2WPluginBase* plugin)
: QMainWindow(nullptr), m_plugin(plugin)
{
  setWindowTitle(QString::fromStdString(plugin->pluginName()));

  QMenuBar* mb = new QMenuBar(this);
  setMenuBar(mb);

  QMenu* fileMenu = new QMenu(tr("&File"), mb);
  fileMenu->addAction(tr("&Open..."), this, SLOT(openFile()));
  fileMenu->addSeparator();
  fileMenu->addAction(tr("E&xit"), qApp, SLOT(quit()));
  mb->addMenu(fileMenu);

  QMenu* helpMenu = new QMenu(tr("&Help"), mb);
  helpMenu->addAction(tr("&About..."), this, SLOT(about()));
  helpMenu->addAction(tr("About &Qt..."), qApp, SLOT(aboutQt()));
  mb->addMenu(helpMenu);
}

void MainWindow::openFile()
{
  QStringList filters;
  for (const auto& pair : m_plugin->extensions()) {
    QString desc = QString::fromStdString(pair.second);
    if (!desc.contains("(")) {
      desc += QStringLiteral(" (*.%1)").arg(QString::fromStdString(pair.first));
    }
    filters << desc;
  }
  QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), QString(), filters.join(";;"));
  if (filename.isEmpty()) {
    return;
  }
  std::string stdFilename = filename.toStdString();
  auto stream = m_plugin->context()->openFile(stdFilename);
  ctx = m_plugin->prepare(stdFilename, *stream);
}

void MainWindow::about()
{
  QMessageBox::about(this, QString::fromStdString(m_plugin->pluginName()), QString::fromStdString(m_plugin->about()));
}
