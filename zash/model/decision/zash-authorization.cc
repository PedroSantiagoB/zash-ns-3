#include "zash-authorization.h"

namespace ns3 {

AuthorizationComponent::AuthorizationComponent ()
{
}
AuthorizationComponent::AuthorizationComponent (ConfigurationComponent *c, OntologyComponent *o,
                                                ContextComponent *ctx, ActivityComponent *a,
                                                ConflictComponent *conf, NotificationComponent *n, AuditComponent *adt)
{
  configurationComponent = c;
  ontologyComponent = o;
  contextComponent = ctx;
  activityComponent = a;
  conflictComponent = conf;
  notificationComponent = n;
  auditComponent = adt;
}

void
AuthorizationComponent::processUnauthorized (Request *req)
{
  req->user->rejected.push_back (req->currentDate);
  *auditComponent->zashOutput << "User have now " << req->user->rejected.size ()
                                               << " rejected requests!" << endl;
  if (req->user->rejected.size () >
      (size_t) configurationComponent->blockThreshold)
    {
      auditComponent->blocks.push_back (new AuditEvent (req));
      req->user->blocked = true;
      *auditComponent->zashOutput << "User " << req->user->id << " is blocked!"
                                                   << endl;
      notificationComponent->alertUsers (req->user);
    }
  ++auditComponent->reqDenied;
  *auditComponent->zashOutput << "Request is NOT authorized!" << endl;
}

bool
AuthorizationComponent::authorizeRequest (Request *req,
                                          function<bool (Request *)> explicitAuthentication)
{
  *auditComponent->zashOutput << "Authorization Component" << endl;
  *auditComponent->zashOutput << "Processing Request: " << req->id << endl;
  checkUsers (req->currentDate);
  if (req->user->blocked)
    {
      ++auditComponent->reqDenied;
      *auditComponent->zashOutput << "USER IS BLOCKED - Request is NOT authorized!" << endl;
      return false;
    }
  bool denied = false;
  switch (req->validated)
    {
    case 0:
      denied = !ontologyComponent->verifyOntology (req) ||
               !contextComponent->verifyContext (req, explicitAuthentication) ||
               !activityComponent->verifyActivity (req, explicitAuthentication);
      break;
    case 1:
      denied = !contextComponent->verifyContext (req, explicitAuthentication) ||
               !activityComponent->verifyActivity (req, explicitAuthentication);
      break;
    case 2:
      denied = !activityComponent->verifyActivity (req, explicitAuthentication);
      break;
    default:
      break;
    }

  if (!denied && req->validated == 3)
    {
      if (!conflictComponent->processRequest(req))
        {
          ++auditComponent->reqDenied;
          *auditComponent->zashOutput << "CONFLICT DETECTED - Request is NOT authorized!" << endl;
          return false;
        }
      
      ++auditComponent->reqGranted;
      *auditComponent->zashOutput << "Request is authorized!" << endl;
      return true;
    }
  return false;
}

void
AuthorizationComponent::checkUsers (time_t currentDate)
{
  for (User *user : configurationComponent->users)
    {
      remove_if (user->rejected.begin (), user->rejected.end (), [this, currentDate] (time_t time) {
        return difftime (currentDate, time) < configurationComponent->blockInterval;
      });
    }
}

} // namespace ns3
