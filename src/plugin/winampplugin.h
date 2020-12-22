#ifndef S2W_WINAMPPLUGIN_H
#define S2W_WINAMPPLUGIN_H

#include "s2wconfig.h"
#include <windows.h>
#include "in2.h"
#include "wa_ipc.h"
#include "ipc_pe.h"
#include "codec/sampledata.h"
#include "synth/synthcontext.h"
#include "tagmap.h"
#include "utility.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <fstream>
#include <sstream>

#ifdef min
#undef min
#endif

static ConstPairList fileInfoTags = {
  { "title", "Title" },
  { "artist", "Artist" },
  { "album", "Album" },
  { "albumartist", "Alb. Artist" },
  { "year", "Year" },
  { "date", "Date" },
  { "genre", "Genre" },
  { "comment", "Comment" },
};

template <typename S2WPluginInfo>
class S2WModule : public In_Module {
  struct ExtExpand {
    std::vector<char> exts;
    operator char*() { return exts.data(); }

    ExtExpand() {
      for (const auto& iter : S2WPluginInfo::extensions) {
        exts.insert(exts.end(), iter.first.begin(), iter.first.end());
        exts.push_back('\0');
        exts.insert(exts.end(), iter.second.begin(), iter.second.end());
        exts.push_back('\0');
      }
      exts.push_back('\0');
    }
  };

  static S2WPlugin<S2WPluginInfo> plugin;
  static ExtExpand extensions;
public:
  static S2WModule<S2WPluginInfo>* instance() {
    static S2WModule<S2WPluginInfo> module;
    return &module;
  }

  static void config(HWND parent) {}
  static void about(HWND parent) {
    std::wstring titleBar = toUtf16("About " + plugin.pluginName());
    std::wstring about = toUtf16(plugin.about());
    MessageBoxW(parent, about.c_str(), titleBar.c_str(), MB_OK);
  }
  static void init() {}
  static void quit() {}
  static void getFileInfo(const in_char* inFilename, in_char* outTitle, int* outMS) {
    std::string filename = inFilename ? inFilename : "";
    std::string title = filename;
    bool isCurrent = filename.empty();
    if (isCurrent) {
      title = filename = instance()->currentTrack;
      *outMS = instance()->length * 1000;
    }
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    TagMap tagMap(plugin.getTags(filename, file));
    if (!isCurrent) {
      std::string len = tagMap.count("length_seconds_fp") ? tagMap.at("length_seconds_fp") : std::string();
      if (!len.empty()) {
        std::istringstream ss(len);
        double lenFP = 0;
        ss >> lenFP;
        *outMS = lenFP * 1000;
      }
    }
    if (tagMap.count("display_title")) {
      title = tagMap.at("display_title");
    }
    strncpy(outTitle, title.c_str(), GETFILEINFO_TITLE_LENGTH);
  }
  static int infoBox(const in_char *filename, HWND hwndParent) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    TagMap tagMap(plugin.getTags(filename, file));
    std::ostringstream ss;
    ss << "Filename:" << std::endl << filename << std::endl << std::endl;
    if (tagMap.count("length_seconds_fp")) {
      ss << "Duration:\t" << formatDuration(tagMap.at("length_seconds_fp")) << std::endl;
    }
    for (const auto& iter : fileInfoTags) {
      if (iter.first == "year" && tagMap.count("date") && !tagMap.at("date").empty()) {
        continue;
      }
      if (tagMap.count(iter.first) && !tagMap.at(iter.first).empty()) {
        ss << iter.second << ":\t" << tagMap.at(iter.first) << std::endl;
      }
    }
    MessageBoxW(hwndParent, toUtf16(ss.str()).c_str(), L"Track Info", MB_OK);
    return 1;
  }
  static int isOurFile(const in_char *fn)
  {
    std::ifstream file(fn, std::ios::in | std::ios::binary);
    return plugin.isPlayable(fn, file);
  }
  static int play(const in_char* fn) { return instance()->play(std::string(fn)); }
  int play(const std::string& fn)
  {
    try {
      if (thread && running) {
        {
          std::unique_lock<std::mutex> lock(mutex);
          shouldStop = true;
        }
        thread->join();
        running = false;
        thread.reset(nullptr);
      }
      try {
        std::ifstream file(fn, std::ios::in | std::ios::binary);
        length = plugin.length(fn, file);
      } catch (std::exception& e) {
        std::cerr << "Error reading length: " << e.what() << std::endl;
        return 1;
      }
      std::ifstream file(fn, std::ios::in | std::ios::binary);
      bool ok = plugin.play(fn, file);
      if (!ok) {
        plugin.unload();
        return 1;
      }

      shouldStop = false;
      paused = false;
      int sampleRate = plugin.sampleRate();
      int latency = outMod->Open(sampleRate, 2, 16, -1, -1);
      if (latency < 0) {
        plugin.unload();
        return 1;
      }
      SetInfo(sampleRate * 2 * 16 / 1000, sampleRate / 1000, 2, 1);
      SAVSAInit(latency, sampleRate);
      VSASetInfo(sampleRate, 2);
      outMod->SetVolume(-666);
      running = true;
      currentTrack = fn;
      seekPos = -1;
      thread.reset(new std::thread([this, sampleRate]{
        try {
          uint8_t buffer[1200];
          while (true) {
            double localSeekPos = -1;
            {
              std::unique_lock<std::mutex> lock(mutex);
              if (shouldStop) {
                break;
              }
              localSeekPos = seekPos;
              seekPos = -1;
            }
            if (localSeekPos >= 0) {
              plugin.seek(localSeekPos);
              outMod->Flush(plugin.currentTime() * 1000);
            }
            uint32_t ready = outMod->CanWrite();
            if (ready < 1500) {
              Sleep(20);
              continue;
            }
            size_t written = plugin.fillBuffer(buffer, std::min(ready, sizeof(buffer)));
            if (!written) {
              break;
            }
            playPos = plugin.currentTime();
            SAAddPCMData(reinterpret_cast<char*>(buffer), 2, 16, playPos * 1000);
            VSAAddPCMData(reinterpret_cast<char*>(buffer), 2, 16, playPos * 1000);
            if (dsp_isactive()) {
              written = dsp_dosamples(reinterpret_cast<short*>(buffer), written >> 2, 16, 2, sampleRate) * 4;
            }
            outMod->Write(reinterpret_cast<char*>(buffer), written);
          }

          plugin.unload();
        } catch (std::exception& e) {
          std::cerr << "Caught exception on thread: " << e.what() << std::endl;
        }
        outMod->Close();
        running = false;
      }));
    } catch (std::exception& e) {
      std::cerr << "Caught exception: " << e.what() << std::endl;
    }
    return 0;
  }
  static void pause() {
    instance()->paused = true;
    instance()->outMod->Pause(1);
  }
  static void unpause() {
    instance()->paused = false;
    instance()->outMod->Pause(0);
  }
  static int isPaused() {
    return instance()->paused ? 1 : 0;
  }
  static void stop() {
    {
      std::unique_lock<std::mutex> lock(instance()->mutex);
      instance()->shouldStop = true;
    }
    instance()->thread->join();
  }
  static int getLength() { return instance()->length * 1000; }
  static int getOutputTime() { return instance()->_getOutputTime(); }
  int _getOutputTime() {
    return (seekPos >= 0 ? seekPos : playPos) * 1000;
  }
  static void setOutputTime(int time_in_ms) {
    std::unique_lock<std::mutex> lock(instance()->mutex);
    instance()->seekPos = time_in_ms * 0.001;
  }
  static void setVolume(int volume) { instance()->outMod->SetVolume(volume); }
  static void setPan(int pan) { instance()->outMod->SetPan(pan); }
  static void eqSet(int on, char data[10], int preamp) {}

  std::unique_ptr<std::thread> thread;
  std::mutex mutex;
  std::string currentTrack;
  double seekPos, playPos, length;
  bool shouldStop;
  bool running;
  bool paused;

  S2WModule();
};

template <typename S2WPluginInfo>
S2WModule<S2WPluginInfo>::S2WModule()
: In_Module(In_Module{
    IN_VER,
    const_cast<char*>(plugin.pluginShortName().c_str()),
    0,
    0,
    extensions,
    1, // seekable
    1, // uses output
    &S2WModule::config,
    &S2WModule::about,
    &S2WModule::init,
    &S2WModule::quit,
    &S2WModule::getFileInfo,
    &S2WModule::infoBox,
    &S2WModule::isOurFile,
    &S2WModule::play,
    &S2WModule::pause,
    &S2WModule::unpause,
    &S2WModule::isPaused,
    &S2WModule::stop,
    &S2WModule::getLength,
    &S2WModule::getOutputTime,
    &S2WModule::setOutputTime,
    &S2WModule::setVolume,
    &S2WModule::setPan,
    0, 0, 0, 0, 0, 0, 0, 0, 0,  // visualizer parameters
    0, 0,                       // DSP parameters
    &S2WModule::eqSet,
    nullptr, // setInfo
    0 // outMod
  })
{
  shouldStop = false;
  running = false;
  paused = false;
  seekPos = -1;
  playPos = 0;
  length = 0;
}

#define SEQ2WAV_PLUGIN(S2WPluginInfo) \
  template<> S2WPlugin<S2WPluginInfo> S2WModule<S2WPluginInfo>::plugin = S2WPlugin<S2WPluginInfo>(); \
  template<> S2WModule<S2WPluginInfo>::ExtExpand S2WModule<S2WPluginInfo>::extensions = S2WModule<S2WPluginInfo>::ExtExpand(); \
  extern "C" EXPORT In_Module* winampGetInModule2() { return S2WModule<S2WPluginInfo>::instance(); }

#endif
