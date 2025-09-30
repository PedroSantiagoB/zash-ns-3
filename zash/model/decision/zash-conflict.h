#ifndef CONFLICT
#define CONFLICT

#include <algorithm>
#include <ctime>
#include <iostream>
#include <map>
#include <queue>
#include <vector>

using namespace std;

#include "ns3/zash-activity.h"
#include "ns3/zash-audit.h"
#include "ns3/zash-configuration.h"
#include "ns3/zash-context.h"
#include "ns3/zash-ontology.h"
#include "ns3/zash-models.h"
#include "ns3/zash-utils.h"

namespace ns3 {

enum ConflictType {
    NO_CONFLICT = 0,
    CONCURRENT_CONFLICT = 1,
    PARALLEL_CONFLICT = 2
};

class Conflict {
public:
    int id;
    ConflictType type;
    vector<Request*> requests;
    time_t timestamp;
    bool resolved;
    Request* winner;
    
    Conflict(int i, ConflictType t, vector<Request*> reqs, time_t ts);
    
    friend ostream& operator<<(ostream &out, Conflict const &c) {
        out << "Conflict[" << c.id << "," << (c.type == CONCURRENT_CONFLICT ? "CONCURRENT" : "PARALLEL") 
            << "," << c.requests.size() << " requests," << formatTime(c.timestamp) << "]";
        return out;
    }
};


class DeviceInterference {
public:
    int deviceId1;
    int deviceId2;
    float interferenceLevel; 
    
    DeviceInterference(int d1, int d2, float level);
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
  void calculateFinalScore(float ontologyWeight = 0.3, float trustWeight = 0.3, 
                          float activityWeight = 0.2, float contextWeight = 0.2);
  
  friend ostream& operator<<(ostream &out, MultiMetricScore const &score) {
    out << "Score[User:" << score.userId << ",Ont:" << score.ontologyScore 
        << ",Trust:" << score.trustScore << ",Act:" << score.activityScore 
        << ",Ctx:" << score.contextScore << ",Final:" << score.finalScore << "]";
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
    
    queue<Request*> pendingRequests;
    
    map<int, Conflict*> activeConflicts;
    
    map<int, Request*> deviceInUse;
    
    vector<DeviceInterference*> deviceInterferences;
    
    map<int, UserPriority*> userPriorities;
    
    int conflictTimeout;
    int maxConcurrentRequests;
    
  ConflictComponent();
  ConflictComponent(ConfigurationComponent *c, OntologyComponent *o, ContextComponent *ctx, 
                   ActivityComponent *a, AuditComponent *adt);
    
    bool processRequest(Request *req);
    ConflictType detectConflict(Request *req);
    bool resolveConflict(Conflict *conflict);
    
  bool isDeviceInUse(int deviceId);
  Request* findConcurrentConflict(Request *req);
  Request* selectWinnerConcurrent(vector<Request*> requests);
  
  bool hasDeviceInterference(int deviceId1, int deviceId2);
  vector<Request*> findParallelConflicts(Request *req);
  Request* selectWinnerParallel(vector<Request*> requests);
  
  MultiMetricScore calculateMultiMetricScore(Request *req);
  Request* selectWinnerMultiMetric(vector<Request*> requests);
  float calculateOntologyScore(Request *req);
  float calculateTrustScore(Request *req);
  float calculateActivityScore(Request *req);
  float calculateContextScore(Request *req);
    
    void updateUserPriority(int userId, time_t currentTime);
    int getUserPriority(int userId);
    
    void cleanupExpiredConflicts(time_t currentTime);
    void releaseDevice(int deviceId);
    
    void addDeviceInterference(int deviceId1, int deviceId2, float level);
    void setUserPriority(int userId, int priority);
    
    void logConflict(Conflict *conflict);
    void logConflictResolution(Conflict *conflict, Request *winner);
};

} // namespace ns3

#endif

