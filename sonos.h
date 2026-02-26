#ifndef SONOS_H
#define SONOS_H

#include <HTTPClient.h>
#include "config.h"

int sonosGetVolume();
bool sonosSetVolume(int volume);
bool sonosTogglePlayPause();
bool sonosIsConnected();
bool sonosIsPlaying();

#endif // SONOS_H
