#include "Command.h"

void test() {
  Serial.println("Haio");
}

int indexOfChr(char* arr[], char* key) {
  Serial.println(key);
  for (int i = 0; i < 4; i++) {
    Serial.println(arr[i]);
    Serial.println(strcmp(arr[i], key));
    if (!strcmp(arr[i], key)) return i;
  }

  return -1;
}

Command::Command() {
  
}

void Command::handleI2C(int len) {
  int LEN = Wire.read();
  int ind = 0;
  Serial.print("Data len: ");
  Serial.println(LEN);
  
  for (int i = 0; i < LEN; i++) {
    char c = Wire.read();
    if (c != '{' and c != '}' and c != '"' and c != ' ') {
      Serial.print(c);
      data[ind++] = c;
      data[ind] = '\0';
    }
  }
  Serial.print('\n');
  Serial.println(data);

  flushBuffer();
  handleCommand();
}

void Command::flushBuffer() {
  while (Wire.available() > 0) Wire.read();
}

void Command::handleCommand() {
  Serial.println("\nReaded");
  Serial.println(data);
  
  char key[16];
  char val[16];
  
  getKeyValue(key, val);
  Serial.println(key);
  Serial.println(val);
  
  if (!strcmp(key, "RGBs")) {
    int rgb[3];
    getList(val, rgb);
    strip->setFadeS(rgb);
  } else if (!strcmp(key, "RGBe")) {
    int rgb[3];
    getList(val, rgb);
    strip->setFadeE(rgb);
  } else if (!strcmp(key, "length")) {
    strip->setLen(atoi(val));
  } else if (!strcmp(key, "speed")) {
    strip->setDelay(atoi(val));
  } else if (!strcmp(key, "mode")) {
    int i = indexOfChr(modes, val);
    if (i >= 0) {
      strip->mode = i + 1;
    }
    Serial.print("Mode: ");
    Serial.println(i);
    Serial.print("Strip mode: ");
    Serial.println(strip->mode);
    strip->clearTemp();
  } else if (!strcmp(key, "state")) {
    if (!strcmp(val, "true")) {
      strip->start();
    } else if (!strcmp(val, "false")) {
      strip->stop();
    }
  }
  
  key[0] = '\0';
  val[0] = '\0';
  
  memset(key, 0 , sizeof(key));
  memset(val, 0 , sizeof(val));
}

void Command::getKeyValue(char* key, char* value) {
  byte i = 0;
  byte d_i = 0;
  byte param = 0;
  bool started = false;
  
  while(true) {
    if (data[i] == ':') {
      key[d_i] = '\0';
      param++;
      d_i = 0;
    } else if (data[i] == '\0') {
      value[d_i] = '\0';
      break;
    } else {
      if (param == 0) {
        key[d_i++] = data[i];
      } else {
        value[d_i++] = data[i];
      }
    }
    
    i++;
  } 
}

char* Command::split(char cs, char ce) {
  int n = 0;
  static char param[sizeof(data)];
  
  if (ce == '\0') {
    for (int i = indexof(cs)+1; i <= strlen(data); i++) {
      param[n] = data[i];
      n++;
    }
  } else {
    for (int i = indexof(cs); i <= indexof(ce); i++) {
      param[n] = data[i];
      n++;
    }
  }
  
  param[n] = '\0';
  return param;
}

bool Command::subchar(int ns, int ne, const char* comp) {
  char command[sizeof(data)];
  
  if (ns < 16 or ne < 16) {
    command[0] = '\0';
    int n = 0;
    
    for (int i = ns; i < ne; i++) {
      command[n] = data[i];
      n++;
    }
    
    command[n] = '\0';
    
    if (!strcmp(command, comp)) {
      return true;
    } else {
      return false;
    }
    
  } else {
    return false;
  }
}

int Command::indexof(char c) {
  int ind = -1;
  
  for (int i = 0; i < strlen(data); i++) {
    if (data[i] == c) {
      ind = i;
    }
  }
  
  return ind;
}

void Command::removeCharFrom(char* dt, char c) {
  int k = 0;
  byte i = 0;
  
  while (dt[i] != '\0') {
    if (dt[i] != c) {
      dt[k++] = dt[i];
    }
    i++;
  }
  
  dt[k] = '\0';
}

void Command::getList(char* dt, int* list) {
  char* ptr = strtok(dt, "[,]");
  byte i = 0;
  
  while (ptr != NULL) {
    list[i] = atoi(ptr);
    ptr = strtok(NULL, "[,]");
    i++;
  }
  
  memset(dt, 0, sizeof(dt));
  list[i] = -1;
}
