#include "mainwindow.h"
#include "guiutils.h"
#include "tagview.h"
#include "playercontrols.h"
#include "plugin/baseplugin.h"
#include "riffwriter.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QThread>
#include <QTimer>

MainWindow::MainWindow(S2WPluginBase* plugin)
: QMainWindow(nullptr), m_plugin(plugin), m_autoPlay(false)
{
  setWindowTitle(QString::fromStdString(plugin->pluginName()));

  QObject::connect(this, SIGNAL(loadError(QString,QString)), this, SLOT(onLoadError(QString,QString)), Qt::QueuedConnection);
  QTimer::singleShot(0, this, SLOT(initUI()));
}

void MainWindow::initUI()
{
  QMenuBar* mb = new QMenuBar(this);
  setMenuBar(mb);

  QMenu* fileMenu = new QMenu(tr("&File"), mb);
  lockWhileWorking(fileMenu->addAction(tr("&Open..."), this, SLOT(openFile()), QKeySequence("Ctrl+O")));
  lockWhileWorking(fileMenu->addAction(tr("&Export..."), this, SLOT(exportFile())));
  fileMenu->addSeparator();
  int before = fileMenu->actions().length();
  populateFileMenu(fileMenu);
  if (fileMenu->actions().length() != before) {
    fileMenu->addSeparator();
  }
  fileMenu->addAction(tr("E&xit"), qApp, SLOT(quit()));
  mb->addMenu(fileMenu);

  QMenu* helpMenu = new QMenu(tr("&Help"), mb);
  populateHelpMenu(helpMenu);
  if (helpMenu->actions().length()) {
    helpMenu->addSeparator();
  }
  helpMenu->addAction(tr("&About..."), this, SLOT(about()));
  helpMenu->addAction(tr("About &Qt..."), qApp, SLOT(aboutQt()));
  mb->addMenu(helpMenu);

  QWidget* central = new QWidget(this);
  QVBoxLayout* layout = new QVBoxLayout(central);

  tagView = new TagView(central);
  QObject::connect(tagView, SIGNAL(loadSubsong(QString)), this, SLOT(openSubsong(QString)));
  layout->addWidget(tagView, 1);

  controls = new PlayerControls(central);
  controls->setEnabled(false);
  QObject::connect(this, SIGNAL(exportStarted()), controls, SLOT(exportStarted()));
  layout->addWidget(controls);
  lockWhileWorking(controls);

  setCentralWidget(central);
  QTimer::singleShot(0, this, SLOT(createPluginWidget()));
}

void MainWindow::openFile()
{
  if (!lockWork()) {
    return;
  }
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
    unlockWork();
    return;
  }
  openFile(filename, false, false);
}

void MainWindow::openFile(const QString& path, bool autoPlay)
{
  QDir qtPath(path);
  openFile(qtPath.absolutePath(), true, autoPlay);
}

void MainWindow::openFile(const QString& path, bool doAcquire, bool autoPlay)
{
  if (doAcquire && !lockWork()) {
    return;
  }
  m_autoPlay = autoPlay;
  QStringList parts = path.split('?');
  std::string stdPath = path.toStdString();
  std::string stdFilename = parts[0].toStdString();
  tagView->loadTags(
    m_plugin,
    QDir(path).dirName(),
    stdFilename,
    stdPath
  );
  if (parts.size() == 1) {
    QString subsong = tagView->autoSubsong();
    if (!subsong.isEmpty() && subsong != path) {
      openFile(subsong, false, autoPlay);
      return;
    }
  }
  currentFile = path;
  if (ctx) {
    m_plugin->unload();
    ctx = nullptr;
  }
  {
    auto stream = m_plugin->context()->openFile(stdFilename);
    if (!m_plugin->isPlayable(stdFilename, *stream)) {
      // TODO: error
      m_plugin->unload();
      ctx = nullptr;
      unlockWork();
      return;
    }
  }
  QThread* thread = qThreadCreate([this, stdPath, stdFilename]{
    try {
      auto stream = m_plugin->context()->openFile(stdFilename);
      ctx = m_plugin->prepare(stdPath, *stream);
    } catch (std::exception& e) {
      emit loadError(QString::fromStdString(stdPath), QString::fromStdString(e.what()));
    } catch (...) {
      emit loadError(QString::fromStdString(stdFilename), QString());
    }
  });
  QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  QObject::connect(thread, SIGNAL(destroyed()), this, SLOT(unlockWork()));
  if (autoPlay) {
    QObject::connect(thread, SIGNAL(destroyed()), controls, SLOT(play()));
  }
  thread->start();
}

void MainWindow::onLoadError(const QString& filename, const QString& msg)
{
  QString fullMessage = tr("An error occurred while loading %1").arg(filename);
  if (!msg.isEmpty()) {
    fullMessage += "\n\n" + msg;
  }
  QMessageBox::critical(this, QString::fromStdString(m_plugin->pluginName()), fullMessage);
}

void MainWindow::about()
{
  QMessageBox::about(this, QString::fromStdString(m_plugin->pluginName()), QString::fromStdString(m_plugin->about()));
}

bool MainWindow::lockWork()
{
  if (!busy.testAndSetAcquire(false, true)) {
    return false;
  }
  for (QWidget* w : lockWidgets) {
    w->setEnabled(false);
  }
  for (QAction* a : lockActions) {
    a->setEnabled(false);
  }
  controls->setLoading(true);
  return true;
}

void MainWindow::unlockWork()
{
  for (QWidget* w : lockWidgets) {
    w->setEnabled(true);
  }
  for (QAction* a : lockActions) {
    a->setEnabled(true);
  }
  controls->setContext(ctx);
  controls->setLoading(false);
  emit contextUpdated(ctx);
  busy.storeRelease(false);
}

void MainWindow::lockWhileWorking(QWidget* widget)
{
  lockWidgets << widget;
}

void MainWindow::lockWhileWorking(QAction* action)
{
  lockActions << action;
}

void MainWindow::populateFileMenu(QMenu*)
{
  // no-op
}

void MainWindow::populateHelpMenu(QMenu*)
{
  // no-op
}

QWidget* MainWindow::createPluginWidget(QWidget*)
{
  return nullptr;
}

void MainWindow::createPluginWidget()
{
  pluginWidget = createPluginWidget(centralWidget());
  if (pluginWidget) {
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(centralWidget()->layout());
    layout->addWidget(pluginWidget);
    QObject::connect(this, SIGNAL(contextUpdated(SynthContext*)), pluginWidget, SLOT(contextUpdated(SynthContext*)));
  }
}

void MainWindow::openSubsong(const QString& filename)
{
  openFile(filename, controls->isPlaying());
}

void MainWindow::exportFile()
{
  if (!lockWork()) {
    return;
  }
  if (!ctx) {
    unlockWork();
    return;
  }
  QString filename = currentFile;
  int dotPos = filename.lastIndexOf('.');
  int slashPos = filename.lastIndexOf('/');
  if (dotPos > slashPos) {
    filename = filename.left(dotPos);
  }
  filename += ".wav";
  filename = QFileDialog::getSaveFileName(this, tr("Export File"), filename, "WAVE Files (*.wav)");
  if (filename.isEmpty()) {
    unlockWork();
    return;
  }
  RiffWriter* riff = new RiffWriter(ctx->sampleRate, ctx->outputChannels > 1);
  if (!riff->open(filename.toStdString())) {
    unlockWork();
    // TODO: error
    return;
  }
  controls->stop();
  emit exportStarted();
  QThread* thread = qThreadCreate([this, riff]{
    try {
      ctx->save(riff);
    } catch (...) {
      // TODO: error
    }
    riff->close();
    delete riff;
  });
  QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  QObject::connect(thread, SIGNAL(destroyed()), this, SLOT(unlockWork()));
  QObject::connect(thread, SIGNAL(destroyed()), controls, SLOT(exportFinished()));
  thread->start();
}
