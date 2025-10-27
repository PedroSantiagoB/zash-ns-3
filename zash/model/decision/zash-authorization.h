#ifndef AUTHORIZATION
#define AUTHORIZATION

#include <algorithm>
#include <ctime>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <vector>

using namespace std;

#include "ns3/zash-activity.h"
#include "ns3/zash-audit.h"
#include "ns3/zash-configuration.h"
#include "ns3/zash-conflict.h"
#include "ns3/zash-context.h"
#include "ns3/zash-enums.h"
#include "ns3/zash-models.h"
#include "ns3/zash-notification.h"
#include "ns3/zash-ontology.h"

namespace ns3 {

class AuthorizationComponent
{
public:
  ConfigurationComponent *configurationComponent;
  OntologyComponent *ontologyComponent;
  ContextComponent *contextComponent;
  ActivityComponent *activityComponent;
  ConflictComponent *conflictComponent;
  NotificationComponent *notificationComponent;
  AuditComponent *auditComponent;
  AuthorizationComponent ();
  AuthorizationComponent (ConfigurationComponent *c, OntologyComponent *o, ContextComponent *ctx,
                          ActivityComponent *a, ConflictComponent *conf, NotificationComponent *n, AuditComponent *adt);

  void processUnauthorized (Request *req);

  bool authorizeRequest (Request *req, function<bool (Request *)> explicitAuthentication);

  void checkUsers (time_t currentDate);
};
} // namespace ns3

#endif
