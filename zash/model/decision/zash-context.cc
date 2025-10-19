#include "zash-context.h"

namespace ns3 {

TimePercentage::TimePercentage (int t)
{
  time = t;
}

TimeObject::TimeObject (Device *d, enums::Enum *u, enums::Enum *a)
{
  device = d;
  userLevel = u;
  action = a;
}

compareTObj::compareTObj (Request *r) : key (r)
{
}

bool
compareTObj::operator() (TimeObject *t)
{
  return t->device->id == key->device->id && t->userLevel->value == key->user->userLevel->value &&
         t->action->value == key->action->value;
}

compareTimes::compareTimes (int t) : key (t)
{
}

bool
compareTimes::operator() (TimePercentage *t)
{
  return t->time == key;
}

ContextComponent::ContextComponent ()
{
}
ContextComponent::ContextComponent (ConfigurationComponent *c, AuditComponent *a)
{
  configurationComponent = c;
  auditComponent = a;
  for (Device *device : c->devices)
    {
      for (const pair<string, enums::Enum *> ul : configurationComponent->props->UserLevel)
        {
          for (const pair<string, enums::Enum *> act : configurationComponent->props->Action)
            {
              timeProbs.push_back (new TimeObject (device, ul.second, act.second));
      }
    }
  }
}

// static trust calculation based on expected
// for [DeviceClass x Action] and [UserLevel x Action]
// from [AccessWay, Localization, Time, Age, Group]
bool
ContextComponent::verifyContext (Request *req,
                                 function<bool (Request *)> explicitAuthentication)
{
  ++req->validated;
  *auditComponent->zashOutput << "Context Component" << endl;
  // cout << "Context Component" << endl;
  calculateTime (req, req->currentDate);
  checkBuilding (req->currentDate);
  if (isTimeBuilding)
    {
    *auditComponent->zashOutput << "Time probability is still building" << endl;
      req->context->time = configurationComponent->props->TimeClass.at ("UNCOMMON");
  }
  *auditComponent->zashOutput << "Verify context " << *req->context << " with " << *req->user
                              << " at " << formatTime (req->currentDate) << endl;
  int expectedDevice = req->device->deviceClass->weight + req->action->weight;
  int expectedUser = req->user->userLevel->weight + req->action->weight;
  int expected = min (max (expectedDevice, expectedUser), 100);
  int calculated = min (calculateTrust (req->context, req->user), 100);

  // Armazenar valor calculado para reutilização no ConflictComponent
  req->calculatedTrust = calculated;

  *auditComponent->zashOutput << "Trust level is " << calculated
                              << " and expected is " << expected << endl;
  if (calculated < expected)
  {
    auditComponent->contextFail.push_back (new AuditEvent (req));
    *auditComponent->zashOutput << "Trust level is BELOW expected! Requires proof of identity!"
    << endl;
    if (!explicitAuthentication (req))
    {
      return false;
    }
  }
  *auditComponent->zashOutput << "Trust level is ABOVE expected!" << endl;
  return true;
}

// check if markov build time expired
void
ContextComponent::checkBuilding (time_t currentDate)
{
  if (difftime (limitDate, (time_t) (-1)) == 0)
    {
      limitDate = currentDate + configurationComponent->buildInterval * 24 * 60 * 60; // add days
    configurationComponent->isBuilding = true;
    }
  else if (isTimeBuilding && difftime (currentDate, limitDate) > 0)
    {
    isTimeBuilding = false;
    configurationComponent->isBuilding = false;
      *auditComponent->zashOutput << "Time context stopped building probabilities at "
                                  << formatTime (currentDate) << endl;
  }
}

int
ContextComponent::calculateTrust (Context *context, User *user)
{
  return context->accessWay->weight + context->localization->weight + context->time->weight +
         user->age->weight + context->group->weight;
}

void
ContextComponent::calculateTime (Request *req, time_t currentDate)
{
  int time = 0;
  int hour = extractHour (currentDate);
  if (hour >= 6 && hour < 12)
    {
    time = enums::MORNING;
    }
  else if (hour >= 12 && hour < 18)
    {
    time = enums::AFTERNOON;
    }
  else if ((hour >= 18 && hour <= 23) || (hour >= 0 && hour < 6))
    {
    time = enums::NIGHT;
  }

  auto it = find_if (timeProbs.begin (), timeProbs.end (), compareTObj (req));
  if (it != timeProbs.end ())
    {
    TimeObject *timeObj = it[0];
      recalculateProbabilities (timeObj, time);

      auto it2 = find_if (timeObj->times.begin (), timeObj->times.end (), compareTimes (time));
      if (it2 != timeObj->times.end ())
        {
      TimePercentage *timePct = it2[0];
          if (timePct->percentage < 0.3)
            {
              req->context->time = configurationComponent->props->TimeClass.at ("UNCOMMON");
            }
          else
            {
              req->context->time = configurationComponent->props->TimeClass.at ("COMMON");
      }
    }
  }
}

void
ContextComponent::recalculateProbabilities (TimeObject *timeObj, int time)
{
  ++timeObj->totalOcc;
  *auditComponent->zashOutput << timeObj->device->name << " | " << timeObj->action->key << " | "
                              << timeObj->userLevel->key << endl;
  for (TimePercentage *timePct : timeObj->times)
    {
      if (timePct->time == time)
        {
      ++timePct->occurrences;
    }
      timePct->percentage = (float) timePct->occurrences / timeObj->totalOcc;
  }
}

} // namespace ns3
