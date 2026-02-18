#include "sonos.h"

namespace {

String sonosUrl() {
  return String("http://") + SONOS_IP + ":" + SONOS_PORT + "/MediaRenderer/RenderingControl/Control";
}

}  // namespace

int sonosGetVolume() {
  HTTPClient http;
  http.begin(sonosUrl());
  http.addHeader("Content-Type", "text/xml; charset=\"utf-8\"");
  http.addHeader("SOAPACTION", "\"urn:schemas-upnp-org:service:RenderingControl:1#GetVolume\"");

  const char* body =
    "<?xml version=\"1.0\"?>"
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
    "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
    "<s:Body>"
    "<u:GetVolume xmlns:u=\"urn:schemas-upnp-org:service:RenderingControl:1\">"
    "<InstanceID>0</InstanceID>"
    "<Channel>Master</Channel>"
    "</u:GetVolume>"
    "</s:Body>"
    "</s:Envelope>";

  int httpCode = http.POST(body);
  int volume = -1;

  if (httpCode == 200) {
    String response = http.getString();
    int start = response.indexOf("<CurrentVolume>");
    int end = response.indexOf("</CurrentVolume>");
    if (start > 0 && end > start) {
      volume = response.substring(start + 15, end).toInt();
    }
  }
  http.end();
  return volume;
}

bool sonosSetVolume(int volume) {
  if (volume < 0) volume = 0;
  if (volume > 100) volume = 100;

  HTTPClient http;
  http.begin(sonosUrl());
  http.addHeader("Content-Type", "text/xml; charset=\"utf-8\"");
  http.addHeader("SOAPACTION", "\"urn:schemas-upnp-org:service:RenderingControl:1#SetVolume\"");

  String body = String(
    "<?xml version=\"1.0\"?>"
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
    "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
    "<s:Body>"
    "<u:SetVolume xmlns:u=\"urn:schemas-upnp-org:service:RenderingControl:1\">"
    "<InstanceID>0</InstanceID>"
    "<Channel>Master</Channel>"
    "<DesiredVolume>") + volume + String("</DesiredVolume>"
    "</u:SetVolume>"
    "</s:Body>"
    "</s:Envelope>");

  int httpCode = http.POST(body);
  http.end();
  return httpCode == 200;
}
