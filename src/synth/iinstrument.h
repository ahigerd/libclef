#ifndef S2W_IINSTRUMENT_H
#define S2W_IINSTRUMENT_H

#include <memory>
#include "channel.h"
class SequenceEvent;
class BaseNoteEvent;
class ChannelEvent;
class ModulatorEvent;
class AudioNode;
class Channel;

class IInstrument {
public:
  virtual void channelEvent(Channel* channel, std::shared_ptr<ChannelEvent> event);
  virtual Channel::Note* noteEvent(Channel* channel, std::shared_ptr<BaseNoteEvent> event) = 0;
  virtual void modulatorEvent(Channel* channel, std::shared_ptr<ModulatorEvent> event);
  virtual Channel::Note* userEvent(Channel* channel, std::shared_ptr<SequenceEvent> event);
};

class DefaultInstrument : public IInstrument {
public:
  virtual Channel::Note* noteEvent(Channel* channel, std::shared_ptr<BaseNoteEvent> event);

  void applyEnvelope(Channel* channel, Channel::Note* note, std::shared_ptr<BaseNoteEvent> event);
};

#endif
