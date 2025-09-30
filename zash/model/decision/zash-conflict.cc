#include "zash-conflict.h"

namespace ns3 {

Conflict::Conflict(int i, ConflictType t, vector<Request*> reqs, time_t ts) {
    id = i;
    type = t;
    requests = reqs;
    timestamp = ts;
    resolved = false;
    winner = nullptr;
}

DeviceInterference::DeviceInterference(int d1, int d2, float level) {
    deviceId1 = d1;
    deviceId2 = d2;
    interferenceLevel = level;
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

void MultiMetricScore::calculateFinalScore(float ontologyWeight, float trustWeight, 
                                          float activityWeight, float contextWeight) {
  finalScore = (ontologyScore * ontologyWeight) + 
               (trustScore * trustWeight) + 
               (activityScore * activityWeight) + 
               (contextScore * contextWeight);
}

ConflictComponent::ConflictComponent() {
    conflictTimeout = 30;  
    maxConcurrentRequests = 5;
}

ConflictComponent::ConflictComponent(ConfigurationComponent *c, OntologyComponent *o, 
                                   ContextComponent *ctx, ActivityComponent *a, AuditComponent *adt) {
  configurationComponent = c;
  ontologyComponent = o;
  contextComponent = ctx;
  activityComponent = a;
  auditComponent = adt;
  conflictTimeout = 30;
  maxConcurrentRequests = 5;
  
  for (User *user : c->users) {
    int priority = user->userLevel->weight;
    userPriorities[user->id] = new UserPriority(user->id, priority, 0);
  }
}

bool ConflictComponent::processRequest(Request *req) {
    *auditComponent->zashOutput << "Conflict Component - Processing Request: " << req->id << endl;
    *auditComponent->zashOutput << "User " << req->user->id << " is already authorized, checking conflicts..." << endl;
    
    cleanupExpiredConflicts(req->currentDate);
    
    ConflictType conflictType = detectConflict(req);
    
    if (conflictType == NO_CONFLICT) {
        if (req->action->key == "CONTROL" || req->action->key == "MANAGE") {
            deviceInUse[req->device->id] = req;
        }
        updateUserPriority(req->user->id, req->currentDate);
        return true;
    }
    
    vector<Request*> conflictingRequests;
    conflictingRequests.push_back(req);
    
    if (conflictType == CONCURRENT_CONFLICT) {
        Request* concurrentReq = findConcurrentConflict(req);
        if (concurrentReq) {
            conflictingRequests.push_back(concurrentReq);
        }
    } else if (conflictType == PARALLEL_CONFLICT) {
        vector<Request*> parallelReqs = findParallelConflicts(req);
        conflictingRequests.insert(conflictingRequests.end(), parallelReqs.begin(), parallelReqs.end());
    }
    
    static int conflictIdCounter = 1;
    Conflict *conflict = new Conflict(conflictIdCounter++, conflictType, conflictingRequests, req->currentDate);
    activeConflicts[conflict->id] = conflict;
    
    *auditComponent->zashOutput << "Conflict detected between AUTHORIZED users: " << *conflict << endl;
    logConflict(conflict);
    
    bool resolved = resolveConflict(conflict);
    
    if (resolved && conflict->winner) {
        *auditComponent->zashOutput << "Conflict resolved. Winner: Request " << conflict->winner->id << endl;
        logConflictResolution(conflict, conflict->winner);
        
        if (conflict->winner->action->key == "CONTROL" || conflict->winner->action->key == "MANAGE") {
            deviceInUse[conflict->winner->device->id] = conflict->winner;
        }
        
        updateUserPriority(conflict->winner->user->id, req->currentDate);
        
        conflict->resolved = true;
        
        return (req->id == conflict->winner->id);
    }
    
    return false;
}

ConflictType ConflictComponent::detectConflict(Request *req) {
    if (isDeviceInUse(req->device->id)) {
        return CONCURRENT_CONFLICT;
    }
    
    for (auto& pair : deviceInUse) {
        int deviceId = pair.first;
        if (hasDeviceInterference(req->device->id, deviceId)) {
            return PARALLEL_CONFLICT;
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

bool ConflictComponent::isDeviceInUse(int deviceId) {
    return deviceInUse.find(deviceId) != deviceInUse.end();
}

Request* ConflictComponent::findConcurrentConflict(Request *req) {
    auto it = deviceInUse.find(req->device->id);
    if (it != deviceInUse.end()) {
        return it->second;
    }
    return nullptr;
}

Request* ConflictComponent::selectWinnerConcurrent(vector<Request*> requests) {
    if (requests.empty()) return nullptr;
    
    
    Request *winner = requests[0];
    int maxPriority = getUserPriority(winner->user->id);
    int maxActionWeight = winner->action->weight;
    time_t latestTime = winner->currentDate;
    
    for (Request *req : requests) {
        int userPriority = getUserPriority(req->user->id);
        int actionWeight = req->action->weight;
        
        if (userPriority > maxPriority) {
            winner = req;
            maxPriority = userPriority;
            maxActionWeight = actionWeight;
            latestTime = req->currentDate;
        } else if (userPriority == maxPriority) {
            if (actionWeight > maxActionWeight) {
                winner = req;
                maxActionWeight = actionWeight;
                latestTime = req->currentDate;
            } else if (actionWeight == maxActionWeight) {
                if (req->currentDate > latestTime) {
                    winner = req;
                    latestTime = req->currentDate;
                }
            }
        }
    }
    
    return winner;
}

bool ConflictComponent::hasDeviceInterference(int deviceId1, int deviceId2) {
    if (deviceId1 == deviceId2) return false;
    
    for (DeviceInterference *interference : deviceInterferences) {
        if ((interference->deviceId1 == deviceId1 && interference->deviceId2 == deviceId2) ||
            (interference->deviceId1 == deviceId2 && interference->deviceId2 == deviceId1)) {
            return interference->interferenceLevel > 0.5;  
        }
    }
    
    return false;
}

vector<Request*> ConflictComponent::findParallelConflicts(Request *req) {
    vector<Request*> conflicts;
    
    for (auto& pair : deviceInUse) {
        int deviceId = pair.first;
        Request *deviceReq = pair.second;
        
        if (hasDeviceInterference(req->device->id, deviceId)) {
            conflicts.push_back(deviceReq);
        }
    }
    
    return conflicts;
}

Request* ConflictComponent::selectWinnerParallel(vector<Request*> requests) {
    if (requests.empty()) return nullptr;
    
    return selectWinnerConcurrent(requests);
}

void ConflictComponent::updateUserPriority(int userId, time_t currentTime) {
    auto it = userPriorities.find(userId);
    if (it != userPriorities.end()) {
        it->second->lastAccess = currentTime;
    }
}

int ConflictComponent::getUserPriority(int userId) {
    auto it = userPriorities.find(userId);
    if (it != userPriorities.end()) {
        return it->second->priority;
    }
    return 0; 
}

void ConflictComponent::cleanupExpiredConflicts(time_t currentTime) {
    vector<int> toRemove;
    
    for (auto& pair : activeConflicts) {
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

void ConflictComponent::releaseDevice(int deviceId) {
    deviceInUse.erase(deviceId);
}

void ConflictComponent::addDeviceInterference(int deviceId1, int deviceId2, float level) {
    deviceInterferences.push_back(new DeviceInterference(deviceId1, deviceId2, level));
}

void ConflictComponent::setUserPriority(int userId, int priority) {
    auto it = userPriorities.find(userId);
    if (it != userPriorities.end()) {
        it->second->priority = priority;
    } else {
        userPriorities[userId] = new UserPriority(userId, priority, 0);
    }
}

void ConflictComponent::logConflict(Conflict *conflict) {
    *auditComponent->zashOutput << "CONFLICT LOG: " << *conflict << endl;
    for (Request *req : conflict->requests) {
        *auditComponent->zashOutput << "  - Request " << req->id << " (User " << req->user->id 
                                   << ", Device " << req->device->id << ", Action " << req->action->key << ")" << endl;
    }
}

void ConflictComponent::logConflictResolution(Conflict *conflict, Request *winner) {
  *auditComponent->zashOutput << "CONFLICT RESOLUTION: Conflict " << conflict->id 
                             << " resolved. Winner: Request " << winner->id 
                             << " (User " << winner->user->id << ")" << endl;
}
MultiMetricScore ConflictComponent::calculateMultiMetricScore(Request *req) {
  MultiMetricScore score(req->user->id);
  
  score.ontologyScore = 1.0;
  
  score.trustScore = calculateTrustScore(req);
  
  score.activityScore = calculateActivityScore(req);
  
  score.contextScore = calculateContextScore(req);
  
  score.calculateFinalScore(0.0, 0.3, 0.4, 0.3);
  
  *auditComponent->zashOutput << "Multi-Metric Score: " << score << endl;
  
  return score;
}

Request* ConflictComponent::selectWinnerMultiMetric(vector<Request*> requests) {
  if (requests.empty()) return nullptr;
  
  Request *winner = nullptr;
  float maxScore = -1.0;
  
  *auditComponent->zashOutput << "=== MULTI-METRIC CONFLICT RESOLUTION ===" << endl;
  
  for (Request *req : requests) {
    MultiMetricScore score = calculateMultiMetricScore(req);
    
    if (score.finalScore > maxScore) {
      maxScore = score.finalScore;
      winner = req;
    }
  }
  
  *auditComponent->zashOutput << "Winner selected with score: " << maxScore << endl;
  
  return winner;
}

float ConflictComponent::calculateOntologyScore(Request *req) {
  float userLevelScore = (float)req->user->userLevel->weight / 100.0; 
  float actionScore = (float)req->action->weight / 100.0;
  
  return (userLevelScore + actionScore) / 2.0;
}

float ConflictComponent::calculateTrustScore(Request *req) {

  int trustLevel = contextComponent->calculateTrust(req->context, req->user);
  
  return (float)trustLevel / 100.0;
}

float ConflictComponent::calculateActivityScore(Request *req) {

  float activityProb = activityComponent->getUserActivityProbability(req);

  if (activityComponent->isMarkovBuilding) {
    return (float)req->user->userLevel->weight / 100.0;
  }
  
  return activityProb;
}

float ConflictComponent::calculateContextScore(Request *req) {
  float accessWayScore = (float)req->context->accessWay->weight / 100.0;
  float localizationScore = (float)req->context->localization->weight / 100.0;
  float timeScore = (float)req->context->time->weight / 100.0;
  float groupScore = (float)req->context->group->weight / 100.0;
  
  return (accessWayScore + localizationScore + timeScore + groupScore) / 4.0;
}

} // namespace ns3
