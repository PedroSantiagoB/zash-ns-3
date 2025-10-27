#ifndef AUDIT
#define AUDIT

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

#include "ns3/address-utils.h"
#include "ns3/address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/simulator.h"
#include "ns3/zash-models.h"
#include "ns3/zash-utils.h"
#include "ns3/zash-attack.h"
#include "ns3/zash-alteration.h"

using namespace std;

namespace ns3 {

class AuditEvent
{
public:
  Request *request;
  AuditEvent (Request *r);

  friend ostream &
  operator<< (ostream &out, AuditEvent const &a)
  {
    out << formatTime (a.request->currentDate) << " - " << *a.request << endl;
    return out;
  }
};

class AuditComponent
{
public:
  vector<AuditEvent *> ontologyFail;
  vector<AuditEvent *> contextFail;
  vector<AuditEvent *> activityFail;
  vector<AuditEvent *> blocks;
  vector<AuditEvent *> validProofs;
  vector<AuditEvent *> invalidProofs;

  int reqNumber = 0;
  int reqGranted = 0;
  int reqDenied = 0;

  // Metrics
  int adminNumber = 0;
  int criticalNumber = 0;
  int privacyRisk;

  double accessControlRT = 0.0;
  double accessControlRTProof = 0.0;
  double accessControlRTNoProof = 0.0;
  double accessControlRTBlock = 0.0;

  int userLevelNumber = 0;
  int deviceClassNumber = 0;
  int actionNumber = 0;
  int resourceIsolation;

  int minDeviceNumber = 0;
  int maxDeviceNumber = 0;
  int deviceExtensibility;

  int requestedAccessWayNumber = 0;
  int homeAccessWayNumber = 0;
  int personalAccessWayNumber = 0;
  int reqWithNoIntermediaryNumber = 0;
  int reqWithIntermediaryNumber = 0;
  double accessControlEnforcement;

  int accessControlDistance = 2;
  double spatialTemporalLocality;

  int deniedImpersonations = 0;
  int totalImpersonations = 0;
  int deniedAttBuilding = 0;
  int successAttBuilding = 0;
  int successAttProof = 0;
  int deniedAttBlock = 0;

  map<string, int> attSucUl;
  map<string, int> attSucAct;
  map<string, int> attSucDc;
  map<string, int> attSucTime;
  map<string, int> attSucLoc;
  map<string, int> attSucAge;
  map<string, int> attSucGrp;
  map<string, int> attSucAw;
  map<int, int> attSucDev;
  map<int, int> attSucUser;

  map<string, int> attDenUl;
  map<string, int> attDenAct;
  map<string, int> attDenDc;
  map<string, int> attDenTime;
  map<string, int> attDenLoc;
  map<string, int> attDenAge;
  map<string, int> attDenGrp;
  map<string, int> attDenAw;
  map<int, int> attDenDev;
  map<int, int> attDenUser;

  // ===== MÉTRICAS DE CONFLITOS CONCORRENTES =====
  int totalConflicts = 0;
  int conflictsResolved = 0;
  int conflictsFailed = 0;

  int conflictsHierarchical = 0;
  int conflictsMultiMetric = 0;

  int hierarchicalCorrect = 0;
  int hierarchicalTotal = 0;

  int multiMetricCorrect = 0;
  int multiMetricTotal = 0;

  vector<double> conflictResolutionTimes;

  map<int, int> conflictsByDevice;

  map<int, int> conflictWinsByUser;

  map<string, int> conflictsByUserPair; // "User1_User2"

  int falseConflicts = 0;

  string simDate;

  string folderTraces;
  string deviceEnforcerFile;
  string scenarioSimFile;
  string messagesSimFile;
  string metricsSimFile;
  string execSimFile;
  string logSimFile;
  string successAttacksFile;
  string deniedAttacksFile;

  // Create a string stream to store simulation scenario data
  stringstream fileSim;
  // Create a string stream to store simulation messages data
  stringstream fileMsgs;
  // Create a string stream to store simulated executions
  stringstream fileExec;
  // Create a string stream to store main simulation log
  stringstream fileLog;
  // Create a string stream to store successful attacks
  stringstream fileSucAtt;
  // Create a string stream to store denied attacks
  stringstream fileDenAtt;
  // Create a string stream to store each device's messages
  map<string, stringstream *> fileDevs;
  // Create a string stream to store zash output
  stringstream *zashOutput = &fileLog;

  AttackManager *attackManager;
  AlterationManager *alterationManager;

  AuditComponent (string timeLog, string folderTraces);

  /*
   * Print events
   * \param file name of the file
   * \param msg message to append
   */
  void printEvents (vector<AuditEvent *> events, string currDateStr, string type);

  void printEnums (enums::Properties *props);

  /*
   * Append file with given message
   * \param file name of the file
   * \param msg message to append
   */
  void appendFile (string file, string msg);

  /*
   * Device Enforcer callback to store messages exchanged between nodes and
   * server
   * \param path path of callback
   * \param sourceIp IP address from node
   * \param destinyIP IP address from server
   * \param message message exchanged
   */
  void deviceEnforcerCallback (string path, Address sourceIp, Address destinyIP, string message);

  void printDenProf();

  void outputMetrics ();

  void outputConflictMetrics ();

  void countTime (double z_reqTime, bool z_proof, bool isBlocked);

  void storeRequestMetrics (Request *req, enums::Properties *props);

  int calculateTrust (enums::Enum *accessWay, enums::Enum *localization, enums::Enum *time,
                      enums::Enum *age, enums::Enum *group);

  void calculatePossibilities (enums::Properties *props);
};
} // namespace ns3

#endif
