#ifndef SOUND_DECODER_H
#define SOUND_DECODER_H

#include <cstddef>
#include <cstdint>

#include <string>
#include <fstream>

#include "dr_mp3.h"
#include "dr_wav.h"
#include "dr_flac.h"

#include "Buffer.h"

struct stb_vorbis;

class SoundDecoderInterface {
public:
  SoundDecoderInterface(const uint16_t &channels);
  virtual ~SoundDecoderInterface();

  virtual bool seek(const size_t &seekSize) = 0;
  virtual size_t getSamples(char* memory, const size_t &sampleCount) = 0;
  virtual size_t getSamples(Buffer &buffer, const size_t &sampleCount) = 0;

  //std::string path() const { return m_path; }
  uint32_t channels() const;
  uint32_t sampleRate() const;
  uint32_t bitsPerChannel() const;
  size_t size() const;
protected:
  uint16_t m_channels;
  uint32_t m_sampleRate;
  uint32_t m_bitsPerChannel;
  size_t m_size;
};

// 0 - default channels count
// возможно придется самому вычислять моно звук
class MP3Decoder : public SoundDecoderInterface {
public:
  MP3Decoder(const std::string &path, const uint16_t &channels);
  MP3Decoder(const std::string &path, const uint16_t &channels, const size_t &size);
  MP3Decoder(std::ifstream* stream, const uint16_t &channels, const size_t &size = 0);
  MP3Decoder(const char* memory, const size_t &memorySize, const uint16_t &channels, const size_t &size = 0);

  ~MP3Decoder();

  bool seek(const size_t &seekSize) override;
  size_t getSamples(char* memory, const size_t &sampleCount) override;
  size_t getSamples(Buffer &buffer, const size_t &sampleCount) override;
private:
  drmp3 mp3;
};

class WAVDecoder : public SoundDecoderInterface {
public:
  WAVDecoder(const std::string &path, const uint16_t &channels);
  WAVDecoder(const std::string &path, const uint16_t &channels, const size_t &size);
  WAVDecoder(std::ifstream* stream, const uint16_t &channels, const size_t &size = 0);
  WAVDecoder(const char* memory, const size_t &memorySize, const uint16_t &channels, const size_t &size = 0);

  ~WAVDecoder();

  bool seek(const size_t &seekSize) override;
  size_t getSamples(char* memory, const size_t &sampleCount) override;
  size_t getSamples(Buffer &buffer, const size_t &sampleCount) override;
private:
  drwav wav;
};

class FLACDecoder : public SoundDecoderInterface {
public:
  FLACDecoder(const std::string &path, const uint16_t &channels);
  FLACDecoder(const std::string &path, const uint16_t &channels, const size_t &size);
  FLACDecoder(std::ifstream* stream, const uint16_t &channels, const size_t &size);
  FLACDecoder(const char* memory, const size_t &memorySize, const uint16_t &channels, const size_t &size);

  ~FLACDecoder();

  bool seek(const size_t &seekSize) override;
  size_t getSamples(char* memory, const size_t &sampleCount) override;
  size_t getSamples(Buffer &buffer, const size_t &sampleCount) override;
private:
  drflac* flac;
};

class OGGDecoder : public SoundDecoderInterface {
public:
  OGGDecoder(const std::string &path, const uint16_t &channels);
  OGGDecoder(const std::string &path, const uint16_t &channels, const size_t &size);
  OGGDecoder(const char* memory, const size_t &memorySize, const uint16_t &channels, const size_t &size);

  ~OGGDecoder();

  bool seek(const size_t &seekSize) override;
  size_t getSamples(char* memory, const size_t &sampleCount) override;
  size_t getSamples(Buffer &buffer, const size_t &sampleCount) override;
private:
  stb_vorbis* file;
};

int32_t to_al_format(const uint16_t &channels, const uint32_t &bitsPerChannel);

#endif //SOUND_DECODER_H
