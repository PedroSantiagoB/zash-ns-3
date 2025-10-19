#ifndef CONFLICT
#define CONFLICT

#include <algorithm>
#include <ctime>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

#include "ns3/zash-activity.h"
#include "ns3/zash-audit.h"
#include "ns3/zash-configuration.h"
#include "ns3/zash-context.h"
#include "ns3/zash-models.h"
#include "ns3/zash-ontology.h"
#include "ns3/zash-utils.h"

namespace ns3 {

enum ConflictType { NO_CONFLICT = 0, CONCURRENT_CONFLICT = 1 };

class Conflict {
public:
  int id;
  ConflictType type;
  vector<Request *> requests;
  time_t timestamp;
  bool resolved;
  Request *winner;

  Conflict(int i, ConflictType t, vector<Request *> reqs, time_t ts);

  friend ostream &operator<<(ostream &out, Conflict const &c) {
    out << "Conflict[" << c.id << ",CONCURRENT," << c.requests.size()
        << " requests," << formatTime(c.timestamp) << "]";
    return out;
  }
};

class UserPriority {
public:
  int userId;
  int priority;
  time_t lastAccess;

  UserPriority(int uid, int p, time_t last);
};

class MultiMetricScore {
public:
  int userId;
  float ontologyScore;
  float trustScore;
  float activityScore;
  float contextScore;
  float finalScore;

  MultiMetricScore(int uid);

  friend ostream &operator<<(ostream &out, MultiMetricScore const &score) {
    out << "Score[User:" << score.userId << ",Ont:" << score.ontologyScore
        << ",Trust:" << score.trustScore << ",Act:" << score.activityScore
        << ",Ctx:" << score.contextScore << ",Final:" << score.finalScore
        << "]";
    return out;
  }
};

class ConflictComponent {
public:
  ConfigurationComponent *configurationComponent;
  OntologyComponent *ontologyComponent;
  ContextComponent *contextComponent;
  ActivityComponent *activityComponent;
  AuditComponent *auditComponent;

  map<int, Conflict *> activeConflicts;

  map<int, Request *> deviceInUse;
  map<int, time_t> deviceRequestTimestamp;
  map<int, Request *>
      deviceLastRequest; // Armazena a última requisição para cada dispositivo

  map<int, UserPriority *> userPriorities;

  int conflictTimeout; // Timeout geral para resolução de conflitos

  ConflictComponent();
  ConflictComponent(ConfigurationComponent *c, OntologyComponent *o,
                    ContextComponent *ctx, ActivityComponent *a,
                    AuditComponent *adt);

  bool processRequest(Request *req);
  ConflictType detectConflict(Request *req);
  bool resolveConflict(Conflict *conflict);

  Request *findConcurrentConflict(Request *req);

  MultiMetricScore calculateMultiMetricScore(Request *req);
  Request *selectWinnerMultiMetric(vector<Request *> requests);
  float calculateTrustScore(Request *req);
  float calculateActivityScore(Request *req);

  void updateUserPriority(int userId, time_t currentTime);

  void cleanupExpiredConflicts(time_t currentTime);

  void logConflict(Conflict *conflict);
  void logConflictResolution(Conflict *conflict, Request *winner);
};

} // namespace ns3

#endif
