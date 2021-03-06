#ifndef Command_h
#define Command_h

#include "Arduino.h"
#include "LedControl.h"
#include <SoftwareSerial.h>
#include <Wire.h>

class Command {
  public:
    Command();
    char* split(char cs, char ce);
    bool subchar(int ns, int ne, const char* comp);
    int indexof(char c);
    void getList(char* data, int* list);
    void handleI2C(int len);
    class LedControl *strip;
  private:
    void handleCommand();
    void flushBuffer();
    void getKeyValue(char* key, char* value);
    void removeCharFrom(char* dt, char c);
    
    char data[128];
    char* modes[4] = {"trail", "fade", "spectrumctrl", "rainbow"};
    
    bool newData = false;
    bool inProg = false;
    bool rcvInProg = false;
};

#endif
