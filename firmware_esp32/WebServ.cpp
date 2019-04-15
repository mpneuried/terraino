#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <WebServ.h>

WebServ::WebServ(int port){
    AsyncWebServer server(port);
    initRouter()
    
}

void WebServ::notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

AsyncWebServer WebServ::getServer(){
    return server
}

void WebServ::htmlWrp(){
    
}

void WebServ::start(){
    server.begin()
}