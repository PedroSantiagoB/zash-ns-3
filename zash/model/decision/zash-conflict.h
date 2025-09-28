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
    float interferenceLevel;  // 0.0 = sem interferência, 1.0 = interferência total
    
    DeviceInterference(int d1, int d2, float level);
};

class UserPriority {
public:
  int userId;
  int priority;
  time_t lastAccess;
  
  UserPriority(int uid, int p, time_t last);
};

// Estrutura para pontuação multi-métrica (inspirada em KRATOS)
class MultiMetricScore {
public:
  int userId;
  float ontologyScore;      // Pontuação baseada em ontologia (0.0 - 1.0)
  float trustScore;         // Pontuação baseada em confiança (0.0 - 1.0)
  float activityScore;      // Pontuação baseada em atividade do usuário (0.0 - 1.0)
  float contextScore;       // Pontuação baseada em contexto (0.0 - 1.0)
  float finalScore;         // Pontuação final ponderada
  
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
    
    // Lista de interferências entre dispositivos
    vector<DeviceInterference*> deviceInterferences;
    
    // Prioridades dos usuários
    map<int, UserPriority*> userPriorities;
    
    // Configurações
    int conflictTimeout;  // Timeout para resolução de conflitos (segundos)
    int maxConcurrentRequests;  // Máximo de requisições concorrentes
    
  ConflictComponent();
  ConflictComponent(ConfigurationComponent *c, OntologyComponent *o, ContextComponent *ctx, 
                   ActivityComponent *a, AuditComponent *adt);
    
    // Métodos principais
    bool processRequest(Request *req);
    ConflictType detectConflict(Request *req);
    bool resolveConflict(Conflict *conflict);
    
  // Métodos para conflitos concorrentes
  bool isDeviceInUse(int deviceId);
  Request* findConcurrentConflict(Request *req);
  Request* selectWinnerConcurrent(vector<Request*> requests);
  
  // Métodos para conflitos paralelos
  bool hasDeviceInterference(int deviceId1, int deviceId2);
  vector<Request*> findParallelConflicts(Request *req);
  Request* selectWinnerParallel(vector<Request*> requests);
  
  // Métodos para resolução multi-métrica (inspirada em KRATOS)
  MultiMetricScore calculateMultiMetricScore(Request *req);
  Request* selectWinnerMultiMetric(vector<Request*> requests);
  float calculateOntologyScore(Request *req);
  float calculateTrustScore(Request *req);
  float calculateActivityScore(Request *req);
  float calculateContextScore(Request *req);
    
    // Métodos de prioridade
    void updateUserPriority(int userId, time_t currentTime);
    int getUserPriority(int userId);
    
    // Métodos de limpeza
    void cleanupExpiredConflicts(time_t currentTime);
    void releaseDevice(int deviceId);
    
    // Métodos de configuração
    void addDeviceInterference(int deviceId1, int deviceId2, float level);
    void setUserPriority(int userId, int priority);
    
    // Métodos de auditoria
    void logConflict(Conflict *conflict);
    void logConflictResolution(Conflict *conflict, Request *winner);
};

} // namespace ns3

#endif

