#ifndef S2W_VUMETER_H
#define S2W_VUMETER_H

#include <QWidget>
#include <QVector>
#include <QBrush>
#include <QAudio>

class VUMeter : public QWidget
{
Q_OBJECT
public:
  VUMeter(QWidget* parent = nullptr);

  void setScaleMode(QAudio::VolumeScale scale);

public slots:
  void setChannels(int channels);
  void setLevel(int channel, double level);

protected:
  void paintEvent(QPaintEvent*);
  void resizeEvent(QResizeEvent*);

private:
  QVector<double> levels;
  QBrush gradient;
  QAudio::VolumeScale scale;
};

#endif
