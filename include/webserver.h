/* This is a refactorization
 * of nidayand's original webserver
 */

#pragma once

#include <ESP8266WebServer.h>

class WebServer : public ESP8266WebServer
{
public:
  static WebServer &instance()
  {
    static WebServer instance;
    return instance;
  }
  void setup();

  void updatePage(String version, String name);

private:
  WebServer();
};