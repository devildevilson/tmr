#ifndef AL_HELPERS_H
#define AL_HELPERS_H

#include <string>

//struct ALCdevice;
// struct ALCdevice_struct;
// typedef struct ALCdevice_struct ALCdevice;
typedef struct ALCdevice ALCdevice;
typedef int ALenum;

void openalError(const std::string &err);
void openalcError(ALCdevice* device, const std::string &err);
void openalError(const ALenum &error, const std::string &err);
void openalcError(ALCdevice* device, const ALenum &error, const std::string &err);

#endif
