#include "alHelpers.h"

#include "AL/al.h"
#include "AL/alc.h"
// #include "AL/alut.h"
#include "AL/alext.h"

#ifdef _DEBUG
#  include <iostream>
#  include <cassert>
#  define ASSERT(expr) assert(expr)
#else
#  define ASSERT(expr)
#endif

void openalError(const std::string &err) {
#ifdef _DEBUG
  ALenum error = alGetError();
  if (error == AL_NO_ERROR) return;
  
  std::cout << "Error: " << error << " desc: " << alGetString(error) << "\n";
  throw std::runtime_error(err);
#else
  (void)err;
#endif
}

void openalcError(ALCdevice* device, const std::string &err) {
#ifdef _DEBUG
  ALCenum error = alcGetError(device);
  if (error == ALC_NO_ERROR) return;
  
  std::cout << "Error: " << error << " desc: " << alcGetString(device, error) << "\n";
  throw std::runtime_error(err);
#else
  (void)err;
  (void)device;
#endif
}

void openalError(const ALenum &error, const std::string &err) {
  if (error == AL_NO_ERROR) return;

  std::cout << "Error: " << error << " desc: " << alGetString(error) << "\n";
  throw std::runtime_error(err);
}

void openalcError(ALCdevice* device, const ALenum &error, const std::string &err) {
  if (error == AL_NO_ERROR) return;

  std::cout << "Error: " << error << " desc: " << alcGetString(device, error) << "\n";
  throw std::runtime_error(err);
}
