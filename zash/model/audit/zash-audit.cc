#include "zash-audit.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AuditComponent");

AuditEvent::AuditEvent (Request *r)
{
  request = r;
}

AuditComponent::AuditComponent (string timeLog, string ft)
{
  folderTraces = ft;
  simDate = timeLog;
  // Define and assign log files names
  ostringstream convert;
  convert << folderTraces.c_str () << "zash_device_enforcer_traces_" << simDate << ".txt";
  deviceEnforcerFile = convert.str ();
}

inline void
AuditComponent::printEvents (vector<AuditEvent *> events, string currDateStr, string type)
{
  ofstream outfile (type + "_" + currDateStr + ".txt");
  for (AuditEvent *event : events)
    {
      // cout << event->request->id << endl;
      // cout << *event->request->device << endl;
      // cout << *event->request->user << endl;
      // cout << event->request->context->accessWay->key << endl;
      // cout << event->request->context->localization->key << endl;
      // cout << event->request->context->time->key << endl;
      // cout << event->request->context->group->key << endl;
      // cout << *event->request->context << endl;
      // cout << event->request->action->key << endl;
      // cout << event->request->isAttack << endl;
      // cout << *event->request << endl;
      // cout << *event << endl;
      outfile << *event << endl;
    }
  outfile.close ();
}

void
AuditComponent::printEnums (enums::Properties *props)
{
  fileSim << "Enums are:" << endl << endl;
  fileSim << "Action:" << endl;
  for (auto const &a : props->Action)
    {
      fileSim << *a.second << endl;
      attSucAct[a.first] = 0;
      attDenAct[a.first] = 0;
    }

  fileSim << endl << "UserLevel:" << endl;
  for (auto const &a : props->UserLevel)
    {
      fileSim << *a.second << endl;
      attSucUl[a.first] = 0;
      attDenUl[a.first] = 0;
    }

  fileSim << endl << "DeviceClass:" << endl;
  for (auto const &a : props->DeviceClass)
    {
      fileSim << *a.second << endl;
      attSucDc[a.first] = 0;
      attDenDc[a.first] = 0;
    }

  fileSim << endl << "AccessWay:" << endl;
  for (auto const &a : props->AccessWay)
    {
      fileSim << *a.second << endl;
      attSucAw[a.first] = 0;
      attDenAw[a.first] = 0;
    }

  fileSim << endl << "Localization:" << endl;
  for (auto const &a : props->Localization)
    {
      fileSim << *a.second << endl;
      attSucLoc[a.first] = 0;
      attDenLoc[a.first] = 0;
    }

  fileSim << endl << "TimeClass:" << endl;
  for (auto const &a : props->TimeClass)
    {
      fileSim << *a.second << endl;
      attSucTime[a.first] = 0;
      attDenTime[a.first] = 0;
    }

  fileSim << endl << "Age:" << endl;
  for (auto const &a : props->Age)
    {
      fileSim << *a.second << endl;
      attSucAge[a.first] = 0;
      attDenAge[a.first] = 0;
    }

  fileSim << endl << "Group:" << endl;
  for (auto const &a : props->Group)
    {
      fileSim << *a.second << endl;
      attSucGrp[a.first] = 0;
      attDenGrp[a.first] = 0;
    }
}

void
AuditComponent::appendFile (string file, string msg)
{
  ofstream stream;
  stream.open (file);
  stream << msg;
  stream.close ();
}

void
AuditComponent::deviceEnforcerCallback (string path, Address sourceIp, Address destinyIP,
                                        string message)
{
  stringstream sourceIpStr;
  stringstream destinyIpStr;
  if (InetSocketAddress::IsMatchingType (sourceIp))
    {
      sourceIpStr << InetSocketAddress::ConvertFrom (sourceIp).GetIpv4 ();
      destinyIpStr << InetSocketAddress::ConvertFrom (destinyIP).GetIpv4 ();
    }
  else
    {
      sourceIpStr << Inet6SocketAddress::ConvertFrom (sourceIp).GetIpv6 ();
      destinyIpStr << Inet6SocketAddress::ConvertFrom (destinyIP).GetIpv6 ();
    }
  double timeReceived = Simulator::Now ().GetSeconds ();
  ostringstream msg;
  msg << timeReceived << "\t" << sourceIpStr.str () << "\t" << destinyIpStr.str () << "\t"
      << message << endl;

  // // Save all emergency messages sent in only one file
  // appendFile(deviceEnforcerStream, deviceEnforcerFile, msg.str());

  ostringstream convert;
  convert << folderTraces.c_str () << "messages/messages_" << sourceIpStr.str () << ".txt";

  // cout << fileDevs.size () << " " << " "
  //      << fileDevs.count (convert.str ()) << endl;

  // cout << (fileDevs.size () > 0 ? "cu" : "aaaa") << endl;
  // map<string, stringstream *>::iterator it = fileDevs.find (convert.str ());
  // for (auto const &f : fileDevs)
  //   {
  //     cout << f.first << " | " << f.second << endl;
  //   }
  if (fileDevs.count (convert.str ()))
    {
      //element found;
      // stream = it->second;
      //   }

      // if (fileDevs.size () > 0 && fileDevs.count (convert.str ()))
      //   {
      // cout << "appending " << convert.str () << endl;

      *fileDevs[convert.str ()] << msg.str ();
    }
  else
    {
      // cout << "inserting " << convert.str () << endl;
      stringstream *devMsg = new stringstream ();
      *devMsg << msg.str ();
      fileDevs.insert ({convert.str (), devMsg});
      // cout << "inserted " << convert.str () << endl;
    }

  // Save received messages individually by IP address
  // appendFile (convert.str (), msg.str ());
}

void
AuditComponent::printDenProf ()
{
  fileLog << "Denied Impersonations Profile:" << endl;
  for (auto const &x : attDenUl)
    {
      fileLog << "User Level: " << x.first << " = " << x.second << "("
              << percentage (x.second, deniedImpersonations) << ")" << endl;
    }
  for (auto const &x : attDenAct)
    {
      fileLog << "Action: " << x.first << " = " << x.second << "("
              << percentage (x.second, deniedImpersonations) << ")" << endl;
    }
  for (auto const &x : attDenDc)
    {
      fileLog << "Device Class: " << x.first << " = " << x.second << "("
              << percentage (x.second, deniedImpersonations) << ")" << endl;
    }
  for (auto const &x : attDenTime)
    {
      fileLog << "Time: " << x.first << " = " << x.second << "("
              << percentage (x.second, deniedImpersonations) << ")" << endl;
    }
  for (auto const &x : attDenLoc)
    {
      fileLog << "Localization: " << x.first << " = " << x.second << "("
              << percentage (x.second, deniedImpersonations) << ")" << endl;
    }
  for (auto const &x : attDenAge)
    {
      fileLog << "Age: " << x.first << " = " << x.second << "("
              << percentage (x.second, deniedImpersonations) << ")" << endl;
    }
  for (auto const &x : attDenGrp)
    {
      fileLog << "Group: " << x.first << " = " << x.second << "("
              << percentage (x.second, deniedImpersonations) << ")" << endl;
    }
  for (auto const &x : attDenAw)
    {
      fileLog << "Access Way: " << x.first << " = " << x.second << "("
              << percentage (x.second, deniedImpersonations) << ")" << endl;
    }
  for (auto const &x : attDenDev)
    {
      fileLog << "Device: " << x.first << " = " << x.second << "("
              << percentage (x.second, deniedImpersonations) << ")" << endl;
    }
  for (auto const &x : attDenUser)
    {
      fileLog << "User: " << x.first << " = " << x.second << "("
              << percentage (x.second, deniedImpersonations) << ")" << endl;
    }
}

void
AuditComponent::outputMetrics ()
{
  ofstream fileSimRec;
  fileSimRec.open (metricsSimFile);
  fileSimRec << "**** ZASH file ****" << endl << endl;
  fileSimRec << "Date: " << simDate << endl << endl;

  fileSimRec << "Number of admin users (|AU|) = " << adminNumber << endl;
  fileSimRec << "Number of critical devices (|CD|) = " << criticalNumber << endl;
  fileSimRec << "Privacy Risk (PR) = " << privacyRisk << endl << endl;

  fileSimRec << "Access Control Response Time (ACRT) = " << accessControlRT << " ms" << endl;
  fileSimRec << "Access Control Response Time Proof (ACRTP) = " << accessControlRTProof << " ms"
             << endl;
  fileSimRec << "Access Control Response Time No Proof (ACRTNP) = " << accessControlRTNoProof
             << " ms" << endl;
  fileSimRec << "Access Control Response Time Blocked (ACRTB) = " << accessControlRTBlock << " ms"
             << endl
             << endl;

  fileSimRec << "Access Control Distance (ACD) = " << accessControlDistance << " hops" << endl
             << endl;

  fileSimRec << "Spatial and Temporal Locality (STL) = " << spatialTemporalLocality << endl << endl;

  fileSimRec << "Number of user levels (|UL|) = " << userLevelNumber << endl;
  fileSimRec << "Number of device classes (|DC|) = " << deviceClassNumber << endl;
  fileSimRec << "Number of actions (|A|) = " << actionNumber << endl;
  fileSimRec << "Resources Isolation (RI) = " << resourceIsolation << endl << endl;

  fileSimRec << "Maximum number of devices (Dmax) = " << maxDeviceNumber << endl;
  fileSimRec << "Minimum number of devices (Dmin) = " << minDeviceNumber << endl;
  fileSimRec << "Device Extensibility (DE) = " << deviceExtensibility << endl << endl;

  fileSimRec << "Number of requests of requested access way (|RAC|) = " << requestedAccessWayNumber
             << endl;
  fileSimRec << "Number of requests of home access way (|HAC|) = " << homeAccessWayNumber << endl;
  fileSimRec << "Number of requests of personal access way (|PAC|) = " << personalAccessWayNumber
             << endl;
  fileSimRec << "Number of requests without intermediaries (|NI|) = " << reqWithNoIntermediaryNumber
             << endl;
  fileSimRec << "Number of requests with intermediaries (|I|) = " << reqWithIntermediaryNumber
             << endl;
  fileSimRec << "Access Control Enforcement (ACE) = " << accessControlEnforcement << endl << endl;

  fileSimRec << "Total Impersonations (TI) = " << totalImpersonations << endl;
  fileSimRec << "Successful Impersonations (SI) = " << (totalImpersonations - deniedImpersonations)
             << endl;
  fileSimRec << "Denied Impersonations (DI) = " << deniedImpersonations << endl;
  fileSimRec << "Denied Impersonations Building (DIB) = "
             << percentage (deniedAttBuilding, deniedImpersonations) << endl;
  fileSimRec << "Denied Impersonations Blocked (DIX) = "
             << percentage (deniedAttBlock, deniedImpersonations) << endl;
  fileSimRec << "Successful Impersonations Building (SIB) = "
             << percentage (successAttBuilding, totalImpersonations - deniedImpersonations) << endl;
  fileSimRec << "Successful Impersonations Proof (SIP) = "
             << percentage (successAttProof, totalImpersonations - deniedImpersonations) << endl;
  fileSimRec << "Attacks Avoided Rate (AAR) = "
             << percentage (deniedImpersonations, totalImpersonations) << endl
             << endl;

  fileSimRec << "Denied Impersonations Profile:" << endl;
  for (auto const &x : attDenUl)
    {
      fileSimRec << "User Level: " << x.first << " = " << x.second << "("
                 << percentage (x.second, deniedImpersonations) << ")" << endl;
    }
  for (auto const &x : attDenAct)
    {
      fileSimRec << "Action: " << x.first << " = " << x.second << "("
                 << percentage (x.second, deniedImpersonations) << ")" << endl;
    }
  for (auto const &x : attDenDc)
    {
      fileSimRec << "Device Class: " << x.first << " = " << x.second << "("
                 << percentage (x.second, deniedImpersonations) << ")" << endl;
    }
  for (auto const &x : attDenTime)
    {
      fileSimRec << "Time: " << x.first << " = " << x.second << "("
                 << percentage (x.second, deniedImpersonations) << ")" << endl;
    }
  for (auto const &x : attDenLoc)
    {
      fileSimRec << "Localization: " << x.first << " = " << x.second << "("
                 << percentage (x.second, deniedImpersonations) << ")" << endl;
    }
  for (auto const &x : attDenAge)
    {
      fileSimRec << "Age: " << x.first << " = " << x.second << "("
                 << percentage (x.second, deniedImpersonations) << ")" << endl;
    }
  for (auto const &x : attDenGrp)
    {
      fileSimRec << "Group: " << x.first << " = " << x.second << "("
                 << percentage (x.second, deniedImpersonations) << ")" << endl;
    }
  for (auto const &x : attDenAw)
    {
      fileSimRec << "Access Way: " << x.first << " = " << x.second << "("
                 << percentage (x.second, deniedImpersonations) << ")" << endl;
    }
  for (auto const &x : attDenDev)
    {
      fileSimRec << "Device: " << x.first << " = " << x.second << "("
                 << percentage (x.second, deniedImpersonations) << ")" << endl;
    }
  for (auto const &x : attDenUser)
    {
      fileSimRec << "User: " << x.first << " = " << x.second << "("
                 << percentage (x.second, deniedImpersonations) << ")" << endl;
    }

  fileSimRec << endl << "Successful Impersonations Profile:" << endl;
  for (auto const &x : attSucUl)
    {
      fileSimRec << "User Level: " << x.first << " = " << x.second << "("
                 << percentage (x.second, (totalImpersonations - deniedImpersonations)) << ")"
                 << endl;
    }
  for (auto const &x : attSucAct)
    {
      fileSimRec << "Action: " << x.first << " = " << x.second << "("
                 << percentage (x.second, (totalImpersonations - deniedImpersonations)) << ")"
                 << endl;
    }
  for (auto const &x : attSucDc)
    {
      fileSimRec << "Device Class: " << x.first << " = " << x.second << "("
                 << percentage (x.second, (totalImpersonations - deniedImpersonations)) << ")"
                 << endl;
    }
  for (auto const &x : attSucTime)
    {
      fileSimRec << "Time: " << x.first << " = " << x.second << "("
                 << percentage (x.second, (totalImpersonations - deniedImpersonations)) << ")"
                 << endl;
    }
  for (auto const &x : attSucLoc)
    {
      fileSimRec << "Localization: " << x.first << " = " << x.second << "("
                 << percentage (x.second, (totalImpersonations - deniedImpersonations)) << ")"
                 << endl;
    }
  for (auto const &x : attSucAge)
    {
      fileSimRec << "Age: " << x.first << " = " << x.second << "("
                 << percentage (x.second, (totalImpersonations - deniedImpersonations)) << ")"
                 << endl;
    }
  for (auto const &x : attSucGrp)
    {
      fileSimRec << "Group: " << x.first << " = " << x.second << "("
                 << percentage (x.second, (totalImpersonations - deniedImpersonations)) << ")"
                 << endl;
    }
  for (auto const &x : attSucAw)
    {
      fileSimRec << "Access Way: " << x.first << " = " << x.second << "("
                 << percentage (x.second, (totalImpersonations - deniedImpersonations)) << ")"
                 << endl;
    }
  for (auto const &x : attSucDev)
    {
      fileSimRec << "Device: " << x.first << " = " << x.second << "("
                 << percentage (x.second, (totalImpersonations - deniedImpersonations)) << ")"
                 << endl;
    }
  for (auto const &x : attSucUser)
    {
      fileSimRec << "User: " << x.first << " = " << x.second << "("
                 << percentage (x.second, (totalImpersonations - deniedImpersonations)) << ")"
                 << endl;
    }

  fileSimRec << endl << "REQUESTS NUMBER = " << reqNumber << endl;

  fileSimRec << "REQUESTS GRANTED = " << reqGranted << endl;

  fileSimRec << "REQUESTS DENIED = " << reqDenied << endl;

  int ontologyFailSize = ontologyFail.size ();
  fileSimRec << "ONTOLOGY FAILS = " << ontologyFailSize << " ("
             << percentage (ontologyFailSize, reqNumber) << ")" << endl;

  int contextFailSize = contextFail.size ();
  fileSimRec << "CONTEXT FAILS = " << contextFailSize << " ("
             << percentage (contextFailSize, reqNumber) << ")" << endl;

  int activityFailSize = activityFail.size ();
  fileSimRec << "ACTIVITY FAILS = " << activityFailSize << " ("
             << percentage (activityFailSize, reqNumber) << ")" << endl;

  int validProofsSize = validProofs.size ();
  int invalidProofsSize = invalidProofs.size ();
  fileSimRec << "VALID PROOFS = " << validProofsSize << " ("
             << percentage (validProofsSize, validProofsSize + invalidProofsSize) << ")" << endl;
  fileSimRec << "INVALID PROOFS = " << invalidProofsSize << " ("
             << percentage (invalidProofsSize, validProofsSize + invalidProofsSize) << ")" << endl;

  fileSimRec << "BLOCKS = " << blocks.size () << endl;

  // Close scenario simulation configuration file
  fileSimRec << "**** End of ZASH file ****" << endl;
  fileSimRec.close ();

  printEvents (ontologyFail, simDate, folderTraces + "/zash_ontology_fail");
  printEvents (contextFail, simDate, folderTraces + "/zash_context_fail");
  printEvents (activityFail, simDate, folderTraces + "/zash_activity_fail");
  printEvents (validProofs, simDate, folderTraces + "/zash_valid_proofs");
  printEvents (invalidProofs, simDate, folderTraces + "/zash_invalid_proofs");
  printEvents (blocks, simDate, folderTraces + "/zash_blocks");

  for (auto const &f : fileDevs)
    {
      // cout << f.first << " | " << f.second << endl;
      createFile (f.first, simDate, (*f.second).str ());
    }

  createFile (successAttacksFile, simDate, fileSucAtt.str ());
  createFile (deniedAttacksFile, simDate, fileDenAtt.str ());

  createFile (logSimFile, simDate, fileLog.str ());
}

void
AuditComponent::storeRequestMetrics (Request *req, enums::Properties *props)
{
  if (req->context->accessWay == props->AccessWay.at ("REQUESTED"))
    {
      ++requestedAccessWayNumber;
      ++reqWithNoIntermediaryNumber;
    }
  else if (req->context->accessWay == props->AccessWay.at ("HOUSE"))
    {
      ++homeAccessWayNumber;
      ++reqWithIntermediaryNumber;
    }
  else
    {
      ++personalAccessWayNumber;
      ++reqWithIntermediaryNumber;
    }

  if (reqWithIntermediaryNumber > 0)
    {
      accessControlEnforcement = reqWithNoIntermediaryNumber / (double) reqWithIntermediaryNumber;
    }
}

int
AuditComponent::calculateTrust (enums::Enum *accessWay, enums::Enum *localization,
                                enums::Enum *time, enums::Enum *age, enums::Enum *group)
{
  return accessWay->weight + localization->weight + time->weight + age->weight + group->weight;
}

void
AuditComponent::calculatePossibilities (enums::Properties *props)
{
  double passed = 0;
  double total = 0;
  int highestExpected = 0;
  int lowestExpected = 200;
  int highestCalculated = 0;
  int lowestCalculated = 200;
  for (auto const &x : props->Action)
    {
      for (auto const &y : props->UserLevel)
        {
          for (auto const &z : props->DeviceClass)
            {
              // cout << "----------------------------------------------------------" << endl;
              // cout << "Calculating expected for:" << endl;
              // cout << "[" << x.second->key << "," << y.second->key << "," << z.second->key << "]"
              //      << endl;
              // cout << "Calculating expected for:" << endl;
              // cout << "[" << x.second->weight << "," << y.second->weight << "," << z.second->weight
              //      << "]" << endl;
              int expectedDevice = z.second->weight + x.second->weight;
              int expectedUser = y.second->weight + x.second->weight;
              // cout << "expectedDevice = " << expectedDevice << endl;
              // cout << "expectedUser = " << expectedUser << endl;
              int expected = min (max (expectedDevice, expectedUser), 100);
              // cout << "expected = " << expected << endl;
              if (expected > highestExpected)
                {
                  highestExpected = expected;
                }
              if (expected < lowestExpected)
                {
                  lowestExpected = expected;
                }
              double passedLocal = 0;
              double totalLocal = 0;
              for (auto const &a : props->AccessWay)
                {
                  for (auto const &b : props->Localization)
                    {
                      for (auto const &c : props->TimeClass)
                        {
                          for (auto const &d : props->Age)
                            {
                              for (auto const &e : props->Group)
                                {
                                  // cout << "***************************************" << endl;
                                  // cout << "Calculating trust for:" << endl;
                                  // cout << "[" << a.second->key << "," << b.second->key << ","
                                  //      << c.second->key << "," << d.second->key << ","
                                  //      << e.second->key << "]" << endl;
                                  int calculated =
                                      min (calculateTrust (a.second, b.second, c.second, d.second,
                                                           e.second),
                                           100);
                                  // cout << "calculated = " << calculated << endl;
                                  if (calculated > highestCalculated)
                                    {
                                      highestCalculated = calculated;
                                    }
                                  if (calculated < lowestCalculated)
                                    {
                                      lowestCalculated = calculated;
                                    }
                                  if (calculated < expected)
                                    {
                                      // cout << "NOT PASSED" << endl;
                                    }
                                  else
                                    {
                                      // cout << "PASSED" << endl;
                                      ++passed;
                                      ++passedLocal;
                                    }
                                  ++total;
                                  ++totalLocal;
                                }
                            }
                        }
                    }
                }

              // cout << "Percentage of passed local = " << passedLocal / totalLocal * 100 << endl;
            }
        }
    }
  fileSim << endl << "----------------------------------------------------------" << endl << endl;
  fileSim << "Percentage of passed = " << passed / total * 100 << endl;
  fileSim << "highestExpected = " << highestExpected << endl;
  fileSim << "lowestExpected = " << lowestExpected << endl;
  fileSim << "highestCalculated = " << highestCalculated << endl;
  fileSim << "lowestCalculated = " << lowestCalculated << endl << endl;
}

void
AuditComponent::countTime (double z_reqTime, bool z_proof, bool isBlocked)
{
  double z_respTime = Simulator::Now ().ToDouble (Time::MS);

  double acrt = z_respTime - z_reqTime;

  accessControlRT = (accessControlRT + acrt) / 2.0;
  if (isBlocked)
    {
      accessControlRTBlock = (accessControlRTBlock + acrt) / 2.0;
    }
  else if (z_proof)
    {
      accessControlRTProof = (accessControlRTProof + acrt) / 2.0;
    }
  else
    {
      accessControlRTNoProof = (accessControlRTNoProof + acrt) / 2.0;
    }
  spatialTemporalLocality = accessControlDistance * accessControlRT;
}

void
AuditComponent::outputConflictMetrics ()
{
  ofstream fileSimRec;
  fileSimRec.open (metricsSimFile, ios::app); // Append ao arquivo existente
  
  fileSimRec << endl << "======================================" << endl;
  fileSimRec << "   CONFLICT RESOLUTION METRICS (RF1-RF7)" << endl;
  fileSimRec << "======================================" << endl << endl;
  
  fileSimRec << "--- M1: Conflitos Detectados (RF1) ---" << endl;
  fileSimRec << "Total de conflitos: " << totalConflicts << endl;
  fileSimRec << "Conflitos resolvidos: " << conflictsResolved << endl;
  fileSimRec << "Conflitos falhados: " << conflictsFailed << endl;
  fileSimRec << "Taxa de sucesso: " 
             << (totalConflicts > 0 ? (conflictsResolved / (double) totalConflicts) * 100 : 0)
             << "%" << endl << endl;
  
  fileSimRec << "--- M2: Taxa de Adequacao Hierarquica (TAH) - RF2 ---" << endl;
  fileSimRec << "Conflitos hierarquicos: " << conflictsHierarchical << endl;
  fileSimRec << "Resolucoes corretas: " << hierarchicalCorrect << endl;
  
  double tah = 0;
  if (hierarchicalTotal > 0)
    {
      tah = (hierarchicalCorrect / (double) hierarchicalTotal) * 100;
    }
  
  fileSimRec << "TAH = " << tah << "%" << endl;
  fileSimRec << "Meta (H1): >=90% -> " << (tah >= 90 ? "PASS" : "FAIL") << endl << endl;
  
  fileSimRec << "--- M3: Acuracia Multi-Metrica (AMM) - RF3 ---" << endl;
  fileSimRec << "Conflitos multi-metrica: " << conflictsMultiMetric << endl;
  fileSimRec << "Resolucoes adequadas: " << multiMetricCorrect << endl;
  
  double amm = 0;
  if (multiMetricTotal > 0)
    {
      amm = (multiMetricCorrect / (double) multiMetricTotal) * 100;
    }
  
  fileSimRec << "AMM = " << amm << "%" << endl;
  fileSimRec << "Meta (H2): >=85% -> " << (amm >= 85 ? "PASS" : "FAIL") << endl << endl;
  
  fileSimRec << "--- M4: Tempo de Resolucao (TMR) - RF6 ---" << endl;
  if (!conflictResolutionTimes.empty ())
    {
      double sum = 0;
      double minTime = conflictResolutionTimes[0];
      double maxTime = conflictResolutionTimes[0];
      
      for (double t : conflictResolutionTimes)
        {
          sum += t;
          if (t < minTime)
            minTime = t;
          if (t > maxTime)
            maxTime = t;
        }
      
      double avgTime = sum / conflictResolutionTimes.size ();
      
      fileSimRec << "Tempo medio: " << avgTime << " ms" << endl;
      fileSimRec << "Tempo minimo: " << minTime << " ms" << endl;
      fileSimRec << "Tempo maximo: " << maxTime << " ms" << endl;
      fileSimRec << "Meta (H4): <=100ms -> " << (avgTime <= 100 ? "PASS" : "FAIL") << endl
                 << endl;
    }
  else
    {
      fileSimRec << "Nenhum tempo registrado." << endl << endl;
    }
  
  fileSimRec << "--- M5: Distribuicao por Dispositivo (RF4) ---" << endl;
  for (auto const &x : conflictsByDevice)
    {
      fileSimRec << "Device " << x.first << ": " << x.second << " conflitos" << endl;
    }
  fileSimRec << endl;
  
  fileSimRec << "--- M6: Vitorias por Usuario (RF5) ---" << endl;
  for (auto const &x : conflictWinsByUser)
    {
      fileSimRec << "User " << x.first << ": " << x.second << " vitorias" << endl;
    }
  fileSimRec << endl;
  
  fileSimRec << "--- M7: Conflitos por Par de Usuarios ---" << endl;
  for (auto const &x : conflictsByUserPair)
    {
      fileSimRec << "Users " << x.first << ": " << x.second << " conflitos" << endl;
    }
  fileSimRec << endl;
  
  fileSimRec << "--- M8: Falsos Conflitos (RF7) ---" << endl;
  fileSimRec << "Total de falsos conflitos: " << falseConflicts << endl;
  
  double fcrRate = 0;
  if (totalConflicts > 0)
    {
      fcrRate = (falseConflicts / (double) totalConflicts) * 100;
    }
  
  fileSimRec << "Taxa de falsos conflitos: " << fcrRate << "%" << endl;
  fileSimRec << "Meta (H5): <=15% -> " << (fcrRate <= 15 ? "PASS" : "FAIL") << endl << endl;
  
  fileSimRec << "======================================" << endl;
  fileSimRec << "   VALIDACAO DE REQUISITOS FUNCIONAIS" << endl;
  fileSimRec << "======================================" << endl << endl;
  
  fileSimRec << "RF1 - Deteccao de conflitos: " 
             << (totalConflicts > 0 ? "IMPLEMENTADO" : "FALHOU") << endl;
  fileSimRec << "RF2 - Resolucao hierarquica: " 
             << (conflictsHierarchical > 0 ? "IMPLEMENTADO" : "NAO TESTADO") << endl;
  fileSimRec << "RF3 - Resolucao multi-metrica: " 
             << (conflictsMultiMetric > 0 ? "IMPLEMENTADO" : "NAO TESTADO") << endl;
  fileSimRec << "RF4 - Timeout por dispositivo: CONFIGURADO" << endl;
  fileSimRec << "RF5 - Transparencia: LOGS DETALHADOS" << endl;
  fileSimRec << "RF6 - Tempo de resposta: " 
             << (!conflictResolutionTimes.empty () ? "MEDIDO" : "SEM DADOS") << endl;
  fileSimRec << "RF7 - Minimizar falsos conflitos: MONITORADO" << endl;
  
  fileSimRec << endl << "======================================" << endl << endl;
  
  fileSimRec.close ();
}

} // namespace ns3
