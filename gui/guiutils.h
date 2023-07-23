#ifndef S2W_GUIUTILS_H
#define S2W_GUIUTILS_H

#include <QThread>

#if QT_CONFIG(cxx11_future)
#define qThreadCreate QThread::create
#else
template <typename FN>
class QThreadRunner : public QThread
{
public:
  QThreadRunner(FN fn) : QThread(nullptr), fn(fn) {}

  void run() { fn(); }

  FN fn;
};
#define qThreadCreate new QThreadRunner
#endif

#endif
