
#ifndef WebServ_h
#define WebServ_h

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

class WebServ
{
  public:
    WebServ(int port);
    void notFound(AsyncWebServerRequest *request);
    void start();
    AsyncWebServer getServer();

  private:
    void initRouter();
    int server;
};

#endif