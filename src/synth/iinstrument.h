#ifndef CLEF_IINSTRUMENT_H
#define CLEF_IINSTRUMENT_H

#include <memory>
#include "channel.h"
class SequenceEvent;
struct BaseNoteEvent;
class ChannelEvent;
class ModulatorEvent;
class AudioNode;
class Channel;

class IInstrument {
public:
  virtual std::vector<int32_t> supportedChannelParams() const;
  virtual std::vector<int32_t> supportedNoteParams() const;

  virtual void channelEvent(Channel* channel, std::shared_ptr<ChannelEvent> event);
  virtual Channel::Note* noteEvent(Channel* channel, std::shared_ptr<BaseNoteEvent> event) = 0;
  virtual void modulatorEvent(Channel* channel, std::shared_ptr<ModulatorEvent> event);
  virtual Channel::Note* userEvent(Channel* channel, std::shared_ptr<SequenceEvent> event);

  virtual std::string displayName() const;
};

class DefaultInstrument : public IInstrument {
public:
  virtual Channel::Note* noteEvent(Channel* channel, std::shared_ptr<BaseNoteEvent> event);

  void applyEnvelope(Channel* channel, Channel::Note* note);
};

#endif
