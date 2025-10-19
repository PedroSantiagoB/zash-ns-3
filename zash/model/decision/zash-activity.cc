#include "zash-activity.h"

namespace ns3 {

ActivityComponent::ActivityComponent ()
{
}
ActivityComponent::ActivityComponent (DataComponent *d, ConfigurationComponent *c,
                                      AuditComponent *a)
{
  dataComponent = d;
  configurationComponent = c;
  auditComponent = a;
}

bool
ActivityComponent::verifyActivity (Request *req, function<bool (Request *)> explicitAuthentication)
{
  *auditComponent->zashOutput << "Activity Component" << endl;
  // cout << "Activity Component" << endl;
  vector<int> lastState = dataComponent->lastState;
  vector<int> currentState = dataComponent->currentState;
  // if (lastState.size () != currentState.size ())
  //   {
  //     resetMarkov (currentDate);
  //   }
  checkBuilding (req->currentDate);
  *auditComponent->zashOutput << "Verify activities" << endl;
  *auditComponent->zashOutput << "From: " << vecToStr (lastState) << endl;
  *auditComponent->zashOutput << "To: " << vecToStr (currentState) << endl;
  if (!isMarkovBuilding)
  {
    float prob = markovChain->getProbability (currentState, lastState);

    // Armazenar valor calculado para reutilização no ConflictComponent
    req->calculatedActivity = prob;

    *auditComponent->zashOutput << "Probability = " << prob << endl;
    if (prob < configurationComponent->markovThreshold)
    {
      auditComponent->activityFail.push_back (new AuditEvent (req));
      *auditComponent->zashOutput
          << "Activity is NOT valid! Requires proof of identity!" << endl;
      if (!explicitAuthentication (req))
      {
        return false;
      }
    }
    *auditComponent->zashOutput << "Activity is valid!" << endl;
    }
  else
    {
    *auditComponent->zashOutput << "Markov Chain is still building" << endl;
    // Durante construção, armazenar valor padrão
    req->calculatedActivity = 1.0;
  }
  ++req->validated;
  markovChain->buildTransition (currentState, lastState);
  return true;
}

// check if markov build time expired
void
ActivityComponent::checkBuilding (time_t currentDate)
{
  if (difftime (limitDate, (time_t) (-1)) == 0)
    {
      limitDate = currentDate + configurationComponent->buildInterval * 24 * 60 * 60; // add days
    configurationComponent->isBuilding = true;
    }
  else if (isMarkovBuilding && difftime (currentDate, limitDate) > 0)
    {
    isMarkovBuilding = false;
    configurationComponent->isBuilding = false;
      *auditComponent->zashOutput << "Markov Chain completed building transition matrix at "
                                  << formatTime (currentDate) << endl;
  }
}

void
ActivityComponent::resetMarkov (time_t currentDate)
{
  *auditComponent->zashOutput << "Markov Chains are reset at " << formatTime (currentDate) << endl;
  delete markovChain;
  markovChain = new MarkovChain ();

  for (auto& pair : userMarkovChains) {
    delete pair.second;
    pair.second = new MarkovChain();
  }

  isMarkovBuilding = true;
  limitDate = currentDate + configurationComponent->buildInterval * 24 * 60 * 60; // add days
}

MarkovChain* ActivityComponent::getUserMarkovChain(int userId) {
  if (userMarkovChains.find(userId) == userMarkovChains.end()) {
    userMarkovChains[userId] = new MarkovChain();
  }
  return userMarkovChains[userId];
}

float ActivityComponent::getUserActivityProbability(Request *req) {
  vector<int> lastState = dataComponent->lastState;
  vector<int> currentState = dataComponent->currentState;

  MarkovChain* userMarkov = getUserMarkovChain(req->user->id);
  return userMarkov->getProbability(currentState, lastState);
}

void ActivityComponent::buildUserTransition(Request *req) {
  vector<int> lastState = dataComponent->lastState;
  vector<int> currentState = dataComponent->currentState;

  MarkovChain* userMarkov = getUserMarkovChain(req->user->id);
  userMarkov->buildTransition(currentState, lastState);
}

} // namespace ns3
