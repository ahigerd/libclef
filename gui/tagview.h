#ifndef S2W_TAGVIEW_H
#define S2W_TAGVIEW_H

#include <QGroupBox>
#include <QList>
#include <string>
#include "tagmap.h"
class S2WPluginBase;
class QLabel;
class QFormLayout;
class QComboBox;

class TagView : public QGroupBox
{
Q_OBJECT
public:
  TagView(QWidget* parent = nullptr);

  QString autoSubsong() const;

public slots:
  void loadTags(S2WPluginBase* plugin, const QString& filename, const std::string& stdFilename, const std::string& stdPath);
  void clearTags();

signals:
  void loadSubsong(const QString& path);

private slots:
  void subsongSelected(int index);

private:
  QFormLayout* layout;
  QComboBox* subsong;
  QList<std::string> tagOrder;
};

#endif
