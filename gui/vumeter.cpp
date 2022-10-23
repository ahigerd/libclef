#include "vumeter.h"
#include <QAudio>
#include <QPalette>
#include <QPainter>
#include <QLinearGradient>

VUMeter::VUMeter(QWidget* parent)
: QWidget(parent)
{
  QPalette p = palette();
  p.setBrush(QPalette::Window, Qt::black);
  setPalette(p);

  setAutoFillBackground(true);
  setFixedHeight(24);
  setChannels(2);
}

void VUMeter::setChannels(int channels)
{
  levels.resize(channels);
}

void VUMeter::setLevel(int channel, double level)
{
  if (channel < levels.size()) {
    level = QAudio::convertVolume(level, QAudio::LinearVolumeScale, QAudio::LogarithmicVolumeScale);
    if (level != levels[channel]) {
      levels[channel] = level;
      update();
    }
  }
}

void VUMeter::paintEvent(QPaintEvent*)
{
  QPainter p(this);
  int chans = levels.size();
  int h = (height() - 4) / chans;
  int w = width() - 8;
  int y = 4;
  p.fillRect(rect(), Qt::black);
  for (int i = 0; i < chans; i++) {
    p.fillRect(4, y, w * levels[i], h - 4, gradient);
    y += h;
  }
}

void VUMeter::resizeEvent(QResizeEvent*)
{
  QLinearGradient lg(0, 0, width() - 8, 0);
  lg.setColorAt(0, QColor(0, 255, 0));
  lg.setColorAt(0.25, QColor(0, 255, 0));
  lg.setColorAt(0.75, QColor(255, 255, 0));
  lg.setColorAt(1.0, QColor(255, 0, 0));
  gradient = lg;
}
