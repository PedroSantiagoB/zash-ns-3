#include "zash-conflict.h"
#include "ns3/zash-activity.h"

namespace ns3 {

Conflict::Conflict(int i, ConflictType t, vector<Request *> reqs, time_t ts) {
  id = i;
  type = t;
  requests = reqs;
  timestamp = ts;
  resolved = false;
  winner = nullptr;
}

UserPriority::UserPriority(int uid, int p, time_t last) {
  userId = uid;
  priority = p;
  lastAccess = last;
}

MultiMetricScore::MultiMetricScore(int uid) {
  userId = uid;
  ontologyScore = 0.0;
  trustScore = 0.0;
  activityScore = 0.0;
  contextScore = 0.0;
  finalScore = 0.0;
}

ConflictComponent::ConflictComponent() { conflictTimeout = 30; }

ConflictComponent::ConflictComponent(ConfigurationComponent *c,
                                     OntologyComponent *o,
                                     ContextComponent *ctx,
                                     ActivityComponent *a,
                                     AuditComponent *adt) {
  configurationComponent = c;
  ontologyComponent = o;
  contextComponent = ctx;
  activityComponent = a;
  auditComponent = adt;
  conflictTimeout = 30;

  for (User *user : c->users) {
    int priority = user->userLevel->weight;
    userPriorities[user->id] = new UserPriority(user->id, priority, 0);
  }
}

bool ConflictComponent::processRequest(Request *req) {
  *auditComponent->zashOutput
      << "Conflict Component - Processing Request: " << req->id << endl;
  *auditComponent->zashOutput << "User " << req->user->id
                              << " is already authorized, checking conflicts..."
                              << endl;

  cleanupExpiredConflicts(req->currentDate);

  ConflictType conflictType = detectConflict(req);

  if (conflictType == NO_CONFLICT) {
    // Registrar timestamp da requisição para futuras detecções de conflito
    deviceRequestTimestamp[req->device->id] = req->currentDate;
    deviceLastRequest[req->device->id] = req;

    if (req->action->key == "CONTROL" || req->action->key == "MANAGE") {
      deviceInUse[req->device->id] = req;
    }
    updateUserPriority(req->user->id, req->currentDate);
    *auditComponent->zashOutput << "Request " << req->id
                                << " authorized - Device " << req->device->id
                                << " timestamp registered" << endl;
    return true;
  }

  vector<Request *> conflictingRequests;
  conflictingRequests.push_back(req);

  if (conflictType == CONCURRENT_CONFLICT) {
    // Buscar a requisição anterior que ainda está dentro do timeout
    Request *concurrentReq = findConcurrentConflict(req);
    if (concurrentReq) {
      conflictingRequests.push_back(concurrentReq);
      *auditComponent->zashOutput << "Concurrent conflict detected for device "
                                  << req->device->id << " between requests "
                                  << req->id << " and " << concurrentReq->id
                                  << endl;
    }
  }

  static int conflictIdCounter = 1;
  Conflict *conflict = new Conflict(conflictIdCounter++, conflictType,
                                    conflictingRequests, req->currentDate);
  activeConflicts[conflict->id] = conflict;

  *auditComponent->zashOutput
      << "Conflict detected between AUTHORIZED users: " << *conflict << endl;
  logConflict(conflict);

  bool resolved = resolveConflict(conflict);

  if (resolved && conflict->winner) {
    *auditComponent->zashOutput << "Conflict resolved. Winner: Request "
                                << conflict->winner->id << endl;
    logConflictResolution(conflict, conflict->winner);

    if (conflict->winner->action->key == "CONTROL" ||
        conflict->winner->action->key == "MANAGE") {
      deviceInUse[conflict->winner->device->id] = conflict->winner;
    }

    updateUserPriority(conflict->winner->user->id, req->currentDate);

    conflict->resolved = true;

    return (req->id == conflict->winner->id);
  }

  return false;
}

ConflictType ConflictComponent::detectConflict(Request *req) {
  // Verificar se o dispositivo tem uma requisição recente dentro do timeout
  auto timestampIt = deviceRequestTimestamp.find(req->device->id);
  if (timestampIt != deviceRequestTimestamp.end()) {
    time_t lastRequestTime = timestampIt->second;
    double timeDiff = difftime(req->currentDate, lastRequestTime);

    // Usar timeout específico do dispositivo
    int deviceTimeout = req->device->conflictTimeout;

    // Se ainda está dentro do timeout de conflito concorrente
    if (timeDiff < deviceTimeout) {
      *auditComponent->zashOutput << "Device " << req->device->id << " ("
                                  << req->device->name
                                  << ") has recent request (" << timeDiff
                                  << "s ago, device timeout: " << deviceTimeout
                                  << "s) - CONCURRENT CONFLICT" << endl;
      return CONCURRENT_CONFLICT;
    } else {
      // Timeout expirado, remover timestamp e última requisição antigos
      deviceRequestTimestamp.erase(timestampIt);
      deviceLastRequest.erase(req->device->id);
      *auditComponent->zashOutput
          << "Device " << req->device->id << " (" << req->device->name
          << ") timeout expired (" << timeDiff
          << "s ago, device timeout: " << deviceTimeout << "s) - NO CONFLICT"
          << endl;
    }
  }

  return NO_CONFLICT;
}

bool ConflictComponent::resolveConflict(Conflict *conflict) {
  if (conflict->requests.size() < 2) {
    return false;
  }

  Request *winner = nullptr;

  winner = selectWinnerMultiMetric(conflict->requests);

  if (winner) {
    conflict->winner = winner;
    return true;
  }

  return false;
}

Request *ConflictComponent::findConcurrentConflict(Request *req) {
  // Retornar a última requisição que ainda está dentro do timeout
  auto it = deviceLastRequest.find(req->device->id);
  if (it != deviceLastRequest.end()) {
    return it->second;
  }
  return nullptr;
}

void ConflictComponent::updateUserPriority(int userId, time_t currentTime) {
  auto it = userPriorities.find(userId);
  if (it != userPriorities.end()) {
    it->second->lastAccess = currentTime;
  }
}

void ConflictComponent::cleanupExpiredConflicts(time_t currentTime) {
  vector<int> toRemove;

  for (auto &pair : activeConflicts) {
    Conflict *conflict = pair.second;
    if (difftime(currentTime, conflict->timestamp) > conflictTimeout) {
      toRemove.push_back(pair.first);
    }
  }

  for (int id : toRemove) {
    delete activeConflicts[id];
    activeConflicts.erase(id);
  }
}

void ConflictComponent::logConflict(Conflict *conflict) {
  *auditComponent->zashOutput << "CONFLICT LOG: " << *conflict << endl;
  for (Request *req : conflict->requests) {
    *auditComponent->zashOutput
        << "  - Request " << req->id << " (User " << req->user->id
        << ", Device " << req->device->id << ", Action " << req->action->key
        << ")" << endl;
  }
}

void ConflictComponent::logConflictResolution(Conflict *conflict,
                                              Request *winner) {
  *auditComponent->zashOutput << "CONFLICT RESOLUTION: Conflict "
                              << conflict->id << " resolved. Winner: Request "
                              << winner->id << " (User " << winner->user->id
                              << ")" << endl;
}
MultiMetricScore ConflictComponent::calculateMultiMetricScore(Request *req) {
  MultiMetricScore score(req->user->id);

  // Ontologia já foi verificada antes - não precisa calcular
  score.ontologyScore = 1.0;

  // Calcular confiança do contexto
  score.trustScore = calculateTrustScore(req);

  // Calcular atividade da cadeia de Markov
  score.activityScore = calculateActivityScore(req);

  // Contexto é redundante com confiança - não usar
  score.contextScore = 0.0;

  // Score final = Confiança × Atividade (multiplicação simples)
  score.finalScore = score.trustScore * score.activityScore;

  *auditComponent->zashOutput << "Simplified Score (Trust×Activity): " << score
                              << endl;

  return score;
}

Request *
ConflictComponent::selectWinnerMultiMetric(vector<Request *> requests) {
  if (requests.empty())
    return nullptr;

  *auditComponent->zashOutput << "=== SIMPLIFIED CONFLICT RESOLUTION ==="
                              << endl;

  // Primeiro: Verificar hierarquia por user-level
  Request *highestLevelReq = nullptr;
  int maxUserLevel = -1;

  for (Request *req : requests) {
    int userLevel = req->user->userLevel->weight;
    *auditComponent->zashOutput << "User " << req->user->id
                                << " has level: " << userLevel << endl;

    if (userLevel > maxUserLevel) {
      maxUserLevel = userLevel;
      highestLevelReq = req;
    }
  }

  // Verificar se há empate no user-level mais alto
  vector<Request *> sameLevelRequests;
  for (Request *req : requests) {
    if (req->user->userLevel->weight == maxUserLevel) {
      sameLevelRequests.push_back(req);
    }
  }

  if (sameLevelRequests.size() == 1) {
    // Apenas um usuário com o nível mais alto - ele ganha automaticamente
    *auditComponent->zashOutput << "Winner by user-level hierarchy: User "
                                << highestLevelReq->user->id << " (level "
                                << maxUserLevel << ")" << endl;
    return highestLevelReq;
  }

  // Empate no user-level mais alto - usar score (Confiança × Atividade)
  *auditComponent->zashOutput << "Tie in user-level " << maxUserLevel
                              << " - resolving with Trust×Activity score"
                              << endl;

  Request *winner = nullptr;
  float maxScore = -1.0;

  for (Request *req : sameLevelRequests) {
    MultiMetricScore score = calculateMultiMetricScore(req);
    *auditComponent->zashOutput << "User " << req->user->id
                                << " score: " << score.finalScore << endl;

    if (score.finalScore > maxScore) {
      maxScore = score.finalScore;
      winner = req;
    }
  }

  *auditComponent->zashOutput << "Winner by score: User " << winner->user->id
                              << " with score " << maxScore << endl;

  return winner;
}

float ConflictComponent::calculateTrustScore(Request *req) {
  // Reutilizar valor calculado no ContextComponent (evita recalcular)
  if (req->calculatedTrust >= 0) {
    return (float)req->calculatedTrust / 100.0;
  }

  // Caso não tenha sido calculado antes (fallback)
  int trustLevel = contextComponent->calculateTrust(req->context, req->user);
  return (float)trustLevel / 100.0;
}

float ConflictComponent::calculateActivityScore(Request *req) {
  // Reutilizar valor calculado no ActivityComponent (evita recalcular)
  if (req->calculatedActivity >= 0.0) {
    return req->calculatedActivity;
  }

  // Caso não tenha sido calculado antes (fallback)
  float activityProb = activityComponent->getUserActivityProbability(req);

  if (activityComponent->isMarkovBuilding) {
    return (float)req->user->userLevel->weight / 100.0;
  }

  return activityProb;
}

} // namespace ns3
