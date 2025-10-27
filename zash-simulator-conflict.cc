/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * ZASH Conflict Resolution Simulator
 *
 * Testa cenários específicos de resolução de conflitos:
 * - Conflitos concorrentes (mesmo dispositivo, timeout)
 * - Resolução hierárquica (user-level)
 * - Resolução por score (trust × activity)
 * - Usuários novos (fallback para cadeia global)
 */

#include "sys/stat.h"
#include "sys/types.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "ns3/core-module.h"
#include "ns3/zash-activity.h"
#include "ns3/zash-audit.h"
#include "ns3/zash-authorization.h"
#include "ns3/zash-configuration.h"
#include "ns3/zash-conflict.h"
#include "ns3/zash-context.h"
#include "ns3/zash-data.h"
#include "ns3/zash-device.h"
#include "ns3/zash-enums.h"
#include "ns3/zash-models.h"
#include "ns3/zash-notification.h"
#include "ns3/zash-ontology.h"
#include "ns3/zash-utils.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("ZASH_CONFLICT");

// Função auxiliar para criar diretório de traces
AuditComponent *createAudit() {
  string tracesFolder;
  string simDate = getTimeOfSimulationStart();

  tracesFolder = "zash_traces_conflict/";
  errno = 0;
  int dir = mkdir(tracesFolder.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  if (dir < 0 && errno != EEXIST)
    cout << "Fail creating directory for traces!" << endl;

  tracesFolder.append(simDate + "/");
  dir = mkdir(tracesFolder.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  if (dir == -1)
    cout << "Fail creating sub directory for specific traces!" << dir << endl;

  AuditComponent *auditModule = new AuditComponent(simDate, tracesFolder);

  ostringstream convert;
  convert << tracesFolder.c_str() << "conflict_test_" << auditModule->simDate
          << ".txt";
  auditModule->scenarioSimFile = convert.str();

  ostringstream convert2;
  convert2 << tracesFolder.c_str() << "zash_simulation_messages_"
           << auditModule->simDate << ".txt";
  auditModule->messagesSimFile = convert2.str();

  ostringstream convert3;
  convert3 << tracesFolder.c_str() << "zash_simulation_metrics_"
           << auditModule->simDate << ".txt";
  auditModule->metricsSimFile = convert3.str();

  ostringstream convert4;
  convert4 << tracesFolder.c_str() << "zash_simulation_simulated_"
           << auditModule->simDate << ".txt";
  auditModule->execSimFile = convert4.str();

  ostringstream convert5;
  convert5 << tracesFolder.c_str() << "zash_log_" << auditModule->simDate
           << ".txt";
  auditModule->logSimFile = convert5.str();

  ostringstream convert6;
  convert6 << tracesFolder.c_str() << "zash_attacks_success_"
           << auditModule->simDate << ".txt";
  auditModule->successAttacksFile = convert6.str();

  ostringstream convert7;
  convert7 << tracesFolder.c_str() << "zash_attacks_denied_"
           << auditModule->simDate << ".txt";
  auditModule->deniedAttacksFile = convert7.str();

  return auditModule;
}

// Função para construir enums
enums::Properties *buildEnums(AuditComponent *auditModule) {
  string enumsConfig = "data/enums_hard.csv";
  auditModule->fileSim << "Enums config file is: " << enumsConfig << endl;
  CsvReader csv(enumsConfig);
  map<string, enums::Enum *> Action;
  map<string, enums::Enum *> UserLevel;
  map<string, enums::Enum *> DeviceClass;
  map<string, enums::Enum *> AccessWay;
  map<string, enums::Enum *> Localization;
  map<string, enums::Enum *> TimeClass;
  map<string, enums::Enum *> Age;
  map<string, enums::Enum *> Group;
  vector<map<string, enums::Enum *>> props = {
      Action,       UserLevel, DeviceClass, AccessWay,
      Localization, TimeClass, Age,         Group};
  vector<vector<string>> propsKeys = {{}, {}, {}, {}, {}, {}, {}, {}};
  int index = 0;
  while (csv.FetchNextRow()) {
    if (csv.ColumnCount() == 0) {
      ++index;
      continue;
    }

    bool ok = true;
    string k;
    int v;
    int w;
    ok |= csv.GetValue(0, k);
    ok |= csv.GetValue(1, v);
    ok |= csv.GetValue(2, w);
    if (!ok) {
      NS_LOG_ERROR("Error parsing csv file, row number: " + csv.RowNumber());
      continue;
    }

    props[index].insert({k, new enums::Enum(k, v, w)});
    propsKeys[index].push_back(k);
  }
  enums::Properties *propsObj =
      new enums::Properties(props[0], props[1], props[2], props[3], props[4],
                            props[5], props[6], props[7]);
  propsObj->actions = propsKeys[0];
  propsObj->userLevels = propsKeys[1];
  propsObj->deviceClasses = propsKeys[2];
  propsObj->accessWays = propsKeys[3];
  propsObj->localizations = propsKeys[4];
  propsObj->timeClasses = propsKeys[5];
  propsObj->ages = propsKeys[6];
  propsObj->groups = propsKeys[7];
  return propsObj;
}

// Função para construir estrutura do sistema ZASH
DeviceComponent *buildZashComponents(AuditComponent *auditModule,
                                     enums::Properties *props) {
  // Criar usuários de diferentes níveis para testar hierarquia
  vector<User *> users = {
      new User(1, props->UserLevel.at("ADMIN"),
               props->Age.at("ADULT")), // Admin
      new User(2, props->UserLevel.at("ADULT"),
               props->Age.at("ADULT")), // Adulto 1
      new User(3, props->UserLevel.at("ADULT"),
               props->Age.at("ADULT")), // Adulto 2
      new User(4, props->UserLevel.at("CHILD"),
               props->Age.at("TEEN")), // Criança 1
      new User(5, props->UserLevel.at("CHILD"),
               props->Age.at("KID")), // Criança 2
      new User(6, props->UserLevel.at("VISITOR"),
               props->Age.at("ADULT")) // Visitante (usuário novo)
  };

  auditModule->fileSim << endl << "Users:" << endl;
  for (User *user : users) {
    auditModule->fileSim << *user << endl;
  }

  // Criar dispositivos com diferentes timeouts para testar conflitos
  vector<Device *> devices = {
      new Device(1, "TV", props->DeviceClass.at("NONCRITICAL"),
                 enums::LIVINGROOM, true),
      new Device(2, "Light", props->DeviceClass.at("NONCRITICAL"),
                 enums::LIVINGROOM, true),
      new Device(3, "AC", props->DeviceClass.at("NONCRITICAL"),
                 enums::LIVINGROOM, true),
      new Device(4, "Door Lock", props->DeviceClass.at("CRITICAL"),
                 enums::HOUSE, true),
      new Device(5, "Security Camera", props->DeviceClass.at("CRITICAL"),
                 enums::HOUSE, true)};

  // Configurar timeouts específicos por dispositivo
  devices[0]->conflictTimeout = 10;  // TV: 10 segundos (ação rápida)
  devices[1]->conflictTimeout = 5;   // Light: 5 segundos (ação instantânea)
  devices[2]->conflictTimeout = 600; // AC: 600 segundos (ação demorada)
  devices[3]->conflictTimeout = 60;  // Door Lock: 60 segundos (ação crítica)
  devices[4]->conflictTimeout =
      300; // Security Camera: 300 segundos (ação crítica)

  auditModule->fileSim << endl << "Devices:" << endl;
  for (Device *device : devices) {
    auditModule->fileSim << *device << " (timeout=" << device->conflictTimeout
                         << "s)" << endl;
  }

  // Criar ontologias (permissões)
  vector<enums::Enum *> visitorCriticalCap = {};
  Ontology *visitorCritical =
      new Ontology(props->UserLevel.at("VISITOR"),
                   props->DeviceClass.at("CRITICAL"), visitorCriticalCap);

  vector<enums::Enum *> childCriticalCap = {props->Action.at("VIEW")};
  Ontology *childCritical =
      new Ontology(props->UserLevel.at("CHILD"),
                   props->DeviceClass.at("CRITICAL"), childCriticalCap);

  vector<enums::Enum *> adultCriticalCap = {props->Action.at("VIEW"),
                                            props->Action.at("CONTROL")};
  Ontology *adultCritical =
      new Ontology(props->UserLevel.at("ADULT"),
                   props->DeviceClass.at("CRITICAL"), adultCriticalCap);

  vector<enums::Enum *> adminCriticalCap = {props->Action.at("VIEW"),
                                            props->Action.at("CONTROL"),
                                            props->Action.at("MANAGE")};
  Ontology *adminCritical =
      new Ontology(props->UserLevel.at("ADMIN"),
                   props->DeviceClass.at("CRITICAL"), adminCriticalCap);

  vector<enums::Enum *> visitorNonCriticalCap = {props->Action.at("VIEW"),
                                                 props->Action.at("CONTROL")};
  Ontology *visitorNonCritical =
      new Ontology(props->UserLevel.at("VISITOR"),
                   props->DeviceClass.at("NONCRITICAL"), visitorNonCriticalCap);

  vector<enums::Enum *> childNonCriticalCap = {props->Action.at("VIEW"),
                                               props->Action.at("CONTROL")};
  Ontology *childNonCritical =
      new Ontology(props->UserLevel.at("CHILD"),
                   props->DeviceClass.at("NONCRITICAL"), childNonCriticalCap);

  vector<enums::Enum *> adultNonCriticalCap = {props->Action.at("VIEW"),
                                               props->Action.at("CONTROL"),
                                               props->Action.at("MANAGE")};
  Ontology *adultNonCritical =
      new Ontology(props->UserLevel.at("ADULT"),
                   props->DeviceClass.at("NONCRITICAL"), adultNonCriticalCap);

  vector<enums::Enum *> adminNonCriticalCap = {props->Action.at("VIEW"),
                                               props->Action.at("CONTROL"),
                                               props->Action.at("MANAGE")};
  Ontology *adminNonCritical =
      new Ontology(props->UserLevel.at("ADMIN"),
                   props->DeviceClass.at("NONCRITICAL"), adminNonCriticalCap);

  vector<Ontology *> ontologies = {
      visitorCritical,    childCritical,    adultCritical,    adminCritical,
      visitorNonCritical, childNonCritical, adultNonCritical, adminNonCritical};

  // Configuração do sistema (Hard mode)
  ConfigurationComponent *configurationComponent = new ConfigurationComponent(
      3, 24, 30, 0.2, devices, users, ontologies, auditModule, props);

  NotificationComponent *notificationComponent =
      new NotificationComponent(configurationComponent, auditModule);

  DataComponent *dataComponent = new DataComponent();

  // Inicializar estado inicial dos dispositivos (todos desligados)
  dataComponent->lastState = {0, 0, 0, 0, 0};
  dataComponent->currentState = {0, 0, 0, 0, 0};

  OntologyComponent *ontologyComponent =
      new OntologyComponent(configurationComponent, auditModule);
  ContextComponent *contextComponent =
      new ContextComponent(configurationComponent, auditModule);
  ActivityComponent *activityComponent =
      new ActivityComponent(dataComponent, configurationComponent, auditModule);

  // Criar o ConflictComponent
  ConflictComponent *conflictComponent =
      new ConflictComponent(configurationComponent, ontologyComponent,
                            contextComponent, activityComponent, auditModule);

  // Criar AuthorizationComponent com ConflictComponent
  AuthorizationComponent *authorizationComponent = new AuthorizationComponent(
      configurationComponent, ontologyComponent, contextComponent,
      activityComponent, conflictComponent, notificationComponent, auditModule);

  DeviceComponent *deviceComponent =
      new DeviceComponent(authorizationComponent, dataComponent, auditModule);

  return deviceComponent;
}

// Função para processar requisição
void processRequest(DeviceComponent *deviceComponent, int deviceId, int userId,
                    string action, time_t currentDate, enums::Properties *props,
                    AuditComponent *auditModule, string scenario) {
  auditModule->fileSim << endl
                       << "========================================" << endl;
  auditModule->fileSim << "SCENARIO: " << scenario << endl;
  auditModule->fileSim << "Time: " << formatTime(currentDate) << endl;
  auditModule->fileSim << "========================================" << endl;

  ConfigurationComponent *config =
      deviceComponent->authorizationComponent->configurationComponent;
  Device *device = config->devices[deviceId];
  User *user = config->users[userId];

  // Criar contexto (interno, sala de estar, sozinho, horário comum)
  Context *context = new Context(
      props->AccessWay.at("REQUESTED"), props->Localization.at("INTERNAL"),
      props->Group.at("ALONE"), props->TimeClass.at("COMMON"));

  enums::Enum *actionEnum = props->Action.at(action);

  Request *req = new Request(deviceId * 100 + userId, device, user, context,
                             actionEnum, 0, currentDate);

  auditModule->zashOutput = &auditModule->fileSim;

  bool authorized = deviceComponent->listenRequest(req);

  auditModule->fileSim << endl
                       << "RESULT: " << (authorized ? "AUTHORIZED" : "DENIED")
                       << endl;
  auditModule->fileSim << "========================================" << endl;
}

int main(int argc, char *argv[]) {
  LogComponentEnable("ZASH_CONFLICT", LOG_LEVEL_ALL);

  cout << "========================================" << endl;
  cout << "ZASH CONFLICT RESOLUTION SIMULATOR" << endl;
  cout << "========================================" << endl << endl;

  AuditComponent *auditModule = createAudit();
  auditModule->fileSim << "ZASH Conflict Resolution Test Scenarios" << endl
                       << endl;

  enums::Properties *props = buildEnums(auditModule);
  DeviceComponent *deviceComponent = buildZashComponents(auditModule, props);

  // Tempo base para os testes
  time_t baseTime = time(NULL);

  auditModule->fileSim << endl
                       << "========================================" << endl;
  auditModule->fileSim << "STARTING CONFLICT TEST SCENARIOS" << endl;
  auditModule->fileSim << "========================================" << endl;

  // ============================================================================
  // CENÁRIO 1: Conflito Concorrente - Mesmo dispositivo, dentro do timeout
  // ============================================================================
  auditModule->fileSim << endl
                       << "### CENÁRIO 1: Conflito Concorrente (TV) ###"
                       << endl;
  auditModule->fileSim
      << "User 2 (Adult) liga TV, User 3 (Adult) tenta desligar 5s depois"
      << endl;
  auditModule->fileSim << "Timeout da TV: 10s - Deve haver conflito!" << endl;

  // User 2 liga TV (t=0)
  processRequest(deviceComponent, 0, 1, "CONTROL", baseTime, props, auditModule,
                 "User 2 (Adult) liga TV");

  // User 3 tenta desligar TV (t=5s) - CONFLITO!
  processRequest(deviceComponent, 0, 2, "CONTROL", baseTime + 5, props,
                 auditModule,
                 "User 3 (Adult) desliga TV após 5s - CONFLITO ESPERADO");

  // ============================================================================
  // CENÁRIO 2: Sem Conflito - Fora do timeout
  // ============================================================================
  auditModule->fileSim << endl
                       << "### CENÁRIO 2: Sem Conflito (TV) ###" << endl;
  auditModule->fileSim << "User 2 liga TV, User 3 tenta desligar 15s depois"
                       << endl;
  auditModule->fileSim << "Timeout da TV: 10s - NÃO deve haver conflito!"
                       << endl;

  // User 2 liga TV (t=20)
  processRequest(deviceComponent, 0, 1, "CONTROL", baseTime + 20, props,
                 auditModule, "User 2 (Adult) liga TV");

  // User 3 desliga TV (t=35s) - SEM CONFLITO
  processRequest(deviceComponent, 0, 2, "CONTROL", baseTime + 35, props,
                 auditModule,
                 "User 3 (Adult) desliga TV após 15s - SEM CONFLITO");

  // ============================================================================
  // CENÁRIO 3: Resolução Hierárquica - Admin vs Adult
  // ============================================================================
  auditModule->fileSim
      << endl
      << "### CENÁRIO 3: Resolução Hierárquica (Admin vs Adult) ###" << endl;
  auditModule->fileSim << "User 1 (Admin) e User 2 (Adult) tentam controlar "
                          "Light simultaneamente"
                       << endl;
  auditModule->fileSim << "Admin deve ganhar automaticamente!" << endl;

  // User 2 (Adult) liga Light (t=50)
  processRequest(deviceComponent, 1, 1, "CONTROL", baseTime + 50, props,
                 auditModule, "User 2 (Adult) liga Light");

  // User 1 (Admin) tenta desligar Light (t=52s) - ADMIN GANHA
  processRequest(deviceComponent, 1, 0, "CONTROL", baseTime + 52, props,
                 auditModule,
                 "User 1 (Admin) desliga Light após 2s - ADMIN DEVE GANHAR");

  // ============================================================================
  // CENÁRIO 4: Resolução por Score - Adultos com mesmo nível
  // ============================================================================
  auditModule->fileSim
      << endl
      << "### CENÁRIO 4: Resolução por Score (Adult vs Adult) ###" << endl;
  auditModule->fileSim << "User 2 e User 3 (ambos Adults) tentam controlar AC"
                       << endl;
  auditModule->fileSim << "Resolução por score (Trust × Activity)" << endl;

  // User 2 liga AC (t=70)
  processRequest(deviceComponent, 2, 1, "CONTROL", baseTime + 70, props,
                 auditModule, "User 2 (Adult) liga AC");

  // User 3 tenta desligar AC (t=75s) - RESOLVIDO POR SCORE
  processRequest(deviceComponent, 2, 2, "CONTROL", baseTime + 75, props,
                 auditModule,
                 "User 3 (Adult) desliga AC após 5s - RESOLVIDO POR SCORE");

  // ============================================================================
  // CENÁRIO 5: Resolução Hierárquica - Child vs Child (por score)
  // ============================================================================
  auditModule->fileSim
      << endl
      << "### CENÁRIO 5: Resolução por Score (Child vs Child) ###" << endl;
  auditModule->fileSim
      << "User 4 e User 5 (ambos Children) tentam controlar Light" << endl;
  auditModule->fileSim << "Resolução por score (Trust × Activity)" << endl;

  // User 4 liga Light (t=100)
  processRequest(deviceComponent, 1, 3, "CONTROL", baseTime + 100, props,
                 auditModule, "User 4 (Child) liga Light");

  // User 5 tenta desligar Light (t=103s) - RESOLVIDO POR SCORE
  processRequest(deviceComponent, 1, 4, "CONTROL", baseTime + 103, props,
                 auditModule,
                 "User 5 (Child) desliga Light após 3s - RESOLVIDO POR SCORE");

  // ============================================================================
  // CENÁRIO 6: Usuário Novo - Fallback para cadeia global
  // ============================================================================
  auditModule->fileSim << endl
                       << "### CENÁRIO 6: Usuário Novo (Fallback) ###" << endl;
  auditModule->fileSim << "User 6 (Visitor - usuário novo) tenta controlar TV"
                       << endl;
  auditModule->fileSim << "Deve usar cadeia global como fallback" << endl;

  // User 6 (novo) liga TV (t=120)
  processRequest(
      deviceComponent, 0, 5, "CONTROL", baseTime + 120, props, auditModule,
      "User 6 (Visitor - NOVO) liga TV - FALLBACK para cadeia global");

  // ============================================================================
  // CENÁRIO 7: Timeout Longo - AC (600s)
  // ============================================================================
  auditModule->fileSim << endl
                       << "### CENÁRIO 7: Timeout Longo (AC) ###" << endl;
  auditModule->fileSim << "User 2 liga AC, User 3 tenta desligar 300s depois"
                       << endl;
  auditModule->fileSim << "Timeout do AC: 600s - Deve haver conflito!" << endl;

  // User 2 liga AC (t=150)
  processRequest(deviceComponent, 2, 1, "CONTROL", baseTime + 150, props,
                 auditModule, "User 2 (Adult) liga AC");

  // User 3 tenta desligar AC (t=450s) - CONFLITO (300s < 600s)
  processRequest(deviceComponent, 2, 2, "CONTROL", baseTime + 450, props,
                 auditModule,
                 "User 3 (Adult) desliga AC após 300s - CONFLITO ESPERADO");

  // ============================================================================
  // CENÁRIO 8: Dispositivo Crítico - Door Lock
  // ============================================================================
  auditModule->fileSim << endl
                       << "### CENÁRIO 8: Dispositivo Crítico (Door Lock) ###"
                       << endl;
  auditModule->fileSim
      << "User 1 (Admin) tranca porta, User 2 (Adult) tenta abrir 30s depois"
      << endl;
  auditModule->fileSim << "Timeout: 60s - Deve haver conflito, Admin ganha"
                       << endl;

  // User 1 (Admin) tranca porta (t=700)
  processRequest(deviceComponent, 3, 0, "CONTROL", baseTime + 700, props,
                 auditModule, "User 1 (Admin) tranca Door Lock");

  // User 2 (Adult) tenta abrir porta (t=730s) - CONFLITO, ADMIN GANHA
  processRequest(
      deviceComponent, 3, 1, "CONTROL", baseTime + 730, props, auditModule,
      "User 2 (Adult) abre Door Lock após 30s - CONFLITO, ADMIN GANHA");

  // ============================================================================
  // Finalização
  // ============================================================================
  auditModule->fileSim << endl
                       << "========================================" << endl;
  auditModule->fileSim << "ALL TEST SCENARIOS COMPLETED" << endl;
  auditModule->fileSim << "========================================" << endl;

  createFile(auditModule->scenarioSimFile, auditModule->simDate,
             auditModule->fileSim.str());
  createFile(auditModule->messagesSimFile, auditModule->simDate,
             auditModule->fileMsgs.str());
  createFile(auditModule->execSimFile, auditModule->simDate,
             auditModule->fileExec.str());

  cout << endl << "========================================" << endl;
  cout << "Test completed! Check output at:" << endl;
  cout << auditModule->scenarioSimFile << endl;
  cout << "========================================" << endl;

  auditModule->outputMetrics();

  return 0;
}
