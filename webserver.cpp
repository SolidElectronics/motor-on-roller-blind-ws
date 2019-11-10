#include "webserver.h"

#include "index_html.h"

namespace {
  void handleRoot()
  {
    WebServer::instance().send(200, "text/html", INDEX_HTML);
  }

  void handleNotFound()
  {
    WebServer& server = WebServer::instance();
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET)?"GET":"POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++){
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
  }
}

WebServer::WebServer()
: ESP8266WebServer(80)
{}

void WebServer::setup()
{
  on("/", handleRoot);
  onNotFound(handleNotFound);
}

void WebServer::updatePage(String version, String name)
{
  INDEX_HTML.replace("{VERSION}", "V"+version);
  INDEX_HTML.replace("{NAME}", name);
}

