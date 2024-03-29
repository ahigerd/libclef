#include "tagview.h"
#include "plugin/baseplugin.h"
#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QtDebug>

static QList<std::string> skipTags{ "display_title", "length_seconds_fp" };

TagView::TagView(QWidget* parent)
: QGroupBox(parent), subsong(nullptr)
{
  tagOrder << "title" << "artist" << "album" << "albumartist";

  layout = new QFormLayout(this);
  clearTags();
}

void TagView::loadTags(ClefPluginBase* plugin, const QString& filename, const std::string& stdFilename, const std::string& stdPath)
{
  clearTags();
  setTitle(filename);

  auto stream = plugin->context()->openFile(stdFilename);
  stream->clear();
  int sampleRate = plugin->sampleRate(stdPath, *stream);

  stream->clear();
  double duration = plugin->length(stdPath, *stream);

  stream->clear();
  TagMap tags = plugin->getTags(stdPath, *stream);

  stream->clear();
  std::vector<std::string> subsongs = plugin->getSubsongs(stdPath, *stream);
  if (subsongs.size() > 0) {
    subsong = new QComboBox(this);
    layout->addRow(tr("&Subsong:"), subsong);
    int target = -1;
    for (const std::string& s : subsongs) {
      std::string subFilename = QString::fromStdString(s).section('?', 0, 0).toStdString();
      try {
        auto subFile = plugin->context()->openFile(subFilename);
        if (!subFile) {
          continue;
        }
        TagMap subTags = plugin->getTags(s, *subFile.get());
        if (s == stdPath) {
          target = subsong->count();
        }
        QString title = QString::fromStdString(subTags["title"]);
        if (title.isEmpty()) {
          title = QString::fromStdString(s).section('?', -1);
        }
        subsong->addItem(title, QString::fromStdString(s));
      } catch (std::exception& e) {
        qDebug() << s.c_str() << subFilename.c_str() << e.what();
        continue;
      }
    }
    subsong->setCurrentIndex(target);
    QObject::connect(subsong, SIGNAL(currentIndexChanged(int)), this, SLOT(subsongSelected(int)), Qt::QueuedConnection);
  }

  int minutes = duration / 60;
  int seconds = duration - (minutes * 60);
  int ms = (duration - (minutes * 60 + seconds)) * 1000;
  if (ms) {
    layout->addRow(tr("Duration:"), new QLabel(QString("%1:%2.%3").arg(minutes).arg(seconds, 2, 10, QChar('0')).arg(ms, 3, 10, QChar('0'))));
  } else {
    layout->addRow(tr("Duration:"), new QLabel(QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'))));
  }
  layout->addRow(tr("Sample Rate:"), new QLabel(QString::number(sampleRate)));
  for (const std::string& tag : tagOrder) {
    auto iter = tags.find(tag);
    if (iter != tags.end()) {
      layout->addRow(QString::fromStdString(iter->first) + ":", new QLabel(QString::fromStdString(iter->second), this));
    }
  }
  for (const auto& iter : tags) {
    if (skipTags.contains(iter.first)) {
      continue;
    }
    if (!tagOrder.contains(iter.first)) {
      layout->addRow(QString::fromStdString(iter.first) + ":", new QLabel(QString::fromStdString(iter.second), this));
      tagOrder << iter.first;
    }
  }
}

void TagView::clearTags()
{
  if (subsong) {
    subsong->deleteLater();
    subsong = nullptr;
  }
  while (layout->rowCount()) {
    layout->removeRow(layout->rowCount() - 1);
  }
  setTitle(tr("(No File Loaded)"));
}

void TagView::subsongSelected(int index)
{
  emit loadSubsong(subsong->itemData(index).toString());
}

QString TagView::autoSubsong() const
{
  if (!subsong || subsong->currentIndex() >= 0) {
    return QString();
  }
  return subsong->itemData(0).toString();
}
