#ifndef SONOS_H
#define SONOS_H

#include <HTTPClient.h>
#include "config.h"

int sonosGetVolume();
bool sonosSetVolume(int volume);

#endif // SONOS_H
