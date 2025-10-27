#ifndef MODELS_ZASH
#define MODELS_ZASH

#include <ctime>
#include <iostream>

using namespace std;

#include "ns3/zash-enums.h"
#include "ns3/zash-utils.h"

namespace ns3 {

class Context {
public:
  enums::Enum *accessWay;
  enums::Enum *localization;
  enums::Enum *group;
  enums::Enum *time;
  Context();
  Context(enums::Enum *aw, enums::Enum *loc, enums::Enum *g, enums::Enum *t);

  friend ostream &operator<<(ostream &out, Context const &c) {
    out << "Context[" << c.accessWay->key << "," << c.localization->key << ","
        << c.time->key << "," << c.group->key << "]";
    return out;
  }
};

class Ontology {
public:
  enums::Enum *userLevel;
  enums::Enum *deviceClass;
  vector<enums::Enum *> capabilities;
  Ontology(enums::Enum *ul, enums::Enum *dc, vector<enums::Enum *> c);

  friend ostream &operator<<(ostream &out, Ontology const &o) {
    out << "Ontology[" << o.userLevel->key << "," << o.deviceClass->key << ","
        << vecToStr(o.capabilities) << "]";
    return out;
  }
};

class User {
public:
  int id;
  enums::Enum *userLevel;
  enums::Enum *age;
  vector<time_t> rejected;
  time_t startInterval;
  bool blocked;
  User();
  User(int i, enums::Enum *ul, enums::Enum *a);

  friend ostream &operator<<(ostream &out, User const &u) {
    out << "User[" << u.id << "," << u.userLevel->key << "," << u.age->key
        << "]";
    return out;
  }
};

class Device {
public:
  int id;
  string name;
  enums::Enum *deviceClass;
  int room;
  bool active;
  bool removed = false;
  int ap;
  int pos;
  int conflictTimeout = 10; // Timeout para conflitos concorrentes (padrão 10s)
  Device();
  Device(int i, string n, enums::Enum *dc, int r, bool a);
  Device(int i, string n, enums::Enum *dc, int r, bool a, int apv, int p);

  friend ostream &operator<<(ostream &out, Device const &d) {
    out << "Device[" << d.id << "," << d.name << "," << d.deviceClass->key
        << "," << d.room << "," << d.removed << "]";
    return out;
  }
};

class Request {
public:
  int id;
  Device *device;
  User *user;
  Context *context;
  enums::Enum *action;
  int attackId;
  time_t currentDate;
  int validated = 0;

  // Valores calculados durante as verificações (para reutilização)
  int calculatedTrust = -1;        // Confiança calculada no ContextComponent
  float calculatedActivity = -1.0; // Atividade calculada no ActivityComponent

  Request();
  Request(int i, Device *d, User *u, Context *c, enums::Enum *a, int at,
          time_t cd);

  friend ostream &operator<<(ostream &out, Request const &r) {
    out << "Request[" << r.id << "," << formatTime(r.currentDate) << ","
        << *r.device << "," << *r.user << "," << *r.context << ","
        << r.action->key << "," << (r.attackId ? "attack" : "normal") << "]";
    return out;
  }
};
} // namespace ns3

#endif
