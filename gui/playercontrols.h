#ifndef S2W_PLAYERCONTROLS_H
#define S2W_PLAYERCONTROLS_H

#include <QWidget>
#include <QAudio>
#include <QTimer>
class QLabel;
class QSlider;
class QToolButton;
class QProgressBar;
class QAudioOutput;
class SynthContext;

class PlayerControls : public QWidget
{
Q_OBJECT
public:
  PlayerControls(QWidget* parent = nullptr);

  bool isPlaying() const;

protected:
  void showEvent(QShowEvent*);
  void resizeEvent(QResizeEvent*);

public slots:
  void setContext(SynthContext* context);
  void seekTo(int ms);
  void play();
  void pause();
  void stop();
  void setLoading(bool loading);
  void exportStarted();
  void exportFinished();

private slots:
  void updateState();
  void togglePlay();
  void stateChanged(QAudio::State state);
  void copyBuffer();

private:
  SynthContext* ctx;
  QSlider* seekBar;
  QProgressBar* loadingBar;
  QLabel* currentTime;
  QToolButton* playButton;
  QToolButton* stopButton;
  QAudioOutput* output;
  QIODevice* stream;
  QByteArray buffer;
  QTimer exportTimer;
};

#endif
