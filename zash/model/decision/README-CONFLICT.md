# Módulo ZASH-Conflict

## Visão Geral

O módulo `zash-conflict` é uma extensão do sistema ZASH que lida com conflitos entre usuários em casas inteligentes. Ele detecta e resolve dois tipos principais de conflitos usando uma abordagem **multi-métrica inspirada em KRATOS**:

1. **Conflitos Concorrentes**: Múltiplos usuários tentando interagir com o mesmo dispositivo simultaneamente
2. **Conflitos Paralelos**: Usuários interagindo com dispositivos diferentes que interferem entre si

## Abordagem Multi-Métrica

O sistema utiliza **3 métricas principais** para resolução de conflitos (após filtro de ontologia):

1. **Confiança** (30%): Calcula nível de confiança baseado no contexto
2. **Atividade** (40%): Analisa padrões de comportamento do usuário (cadeia de Markov)
3. **Contexto** (30%): Considera fatores situacionais (localização, tempo, etc.)

**Nota**: A ontologia atua como **filtro** (não como peso), garantindo que apenas usuários com permissão participem da resolução de conflitos.

## Arquitetura

### Componentes Principais

- **ConflictComponent**: Componente principal que gerencia conflitos
- **Conflict**: Estrutura que representa um conflito específico
- **DeviceInterference**: Define interferências entre dispositivos
- **UserPriority**: Gerencia prioridades dos usuários
- **MultiMetricScore**: Calcula pontuação multi-métrica para resolução de conflitos

### Integração com o Sistema

O módulo se integra com o `AuthorizationComponent` existente, sendo executado **APÓS** as verificações de ontologia, contexto e atividade. Isso garante que apenas usuários já autorizados participem da resolução de conflitos.

**Fluxo Correto:**
```
Requisição → Ontologia → Contexto → Atividade → Conflito → Autorização
```

**Vantagens:**
- **Eficiência**: Não processa conflitos para usuários sem permissão
- **Lógica**: Ontologia atua como filtro, não como peso
- **Segurança**: Apenas usuários autorizados competem

## Tipos de Conflitos

### 1. Conflitos Concorrentes

**Definição**: Dois ou mais usuários tentam interagir com o mesmo dispositivo ao mesmo tempo.

**Exemplo**:
```
Usuário A: Liga TV
Usuário B: Desliga TV (simultaneamente)
```

**Resolução**: Baseada em:
- Prioridade do usuário
- Tipo de ação (MANAGE > CONTROL > VIEW)
- Timestamp da requisição

### 2. Conflitos Paralelos

**Definição**: Usuários interagindo com dispositivos diferentes que interferem entre si.

**Exemplo**:
```
Usuário A: Liga ar condicionado
Usuário B: Liga aquecedor (interferência total)
```

**Resolução**: Baseada em:
- Nível de interferência entre dispositivos
- Prioridade dos usuários
- Compatibilidade das ações

## Configuração

### 1. Interferências entre Dispositivos

```cpp
// Configurar interferência entre dispositivos
conflictComponent->addDeviceInterference(deviceId1, deviceId2, interferenceLevel);

// Exemplos:
conflictComponent->addDeviceInterference(1, 2, 1.0);  // Interferência total
conflictComponent->addDeviceInterference(3, 4, 0.5);  // Interferência média
conflictComponent->addDeviceInterference(5, 6, 0.2);  // Interferência baixa
```

### 2. Prioridades dos Usuários

```cpp
// Configurar prioridade de usuário
conflictComponent->setUserPriority(userId, priority);

// Exemplos:
conflictComponent->setUserPriority(1, 100);  // Admin
conflictComponent->setUserPriority(2, 80);   // Adulto
conflictComponent->setUserPriority(3, 40);   // Criança
```

### 3. Parâmetros do Sistema

```cpp
// Configurar timeout de conflitos
conflictComponent->conflictTimeout = 30;  // 30 segundos

// Configurar máximo de requisições concorrentes
conflictComponent->maxConcurrentRequests = 5;
```

## Algoritmos de Resolução Multi-Métrica

### 1. Cálculo de Pontuação Multi-Métrica

```cpp
MultiMetricScore calculateMultiMetricScore(Request *req) {
    MultiMetricScore score(req->user->id);
    
    // Ontologia (30%): Verifica permissões
    score.ontologyScore = calculateOntologyScore(req);
    
    // Confiança (30%): Nível de confiança baseado no contexto
    score.trustScore = calculateTrustScore(req);
    
    // Atividade (20%): Padrões de comportamento do usuário
    score.activityScore = calculateActivityScore(req);
    
    // Contexto (20%): Fatores situacionais
    score.contextScore = calculateContextScore(req);
    
    // Pontuação final ponderada
    score.calculateFinalScore(0.3, 0.3, 0.2, 0.2);
    
    return score;
}
```

### 2. Seleção de Vencedor Multi-Métrica

```cpp
Request* selectWinnerMultiMetric(vector<Request*> requests) {
    Request *winner = nullptr;
    float maxScore = -1.0;
    
    for (Request *req : requests) {
        MultiMetricScore score = calculateMultiMetricScore(req);
        
        if (score.finalScore > maxScore) {
            maxScore = score.finalScore;
            winner = req;
        }
    }
    
    return winner;
}
```

### 3. Métricas Individuais

#### Ontologia Score
```cpp
float calculateOntologyScore(Request *req) {
    // Verifica se ontologia permite a ação
    bool valid = ontologyComponent->verifyOntology(req);
    if (!valid) return 0.0;
    
    // Combina nível do usuário e tipo de ação
    float userScore = req->user->userLevel->weight / 100.0;
    float actionScore = req->action->weight / 100.0;
    
    return (userScore + actionScore) / 2.0;
}
```

#### Trust Score
```cpp
float calculateTrustScore(Request *req) {
    // Usa o sistema de confiança existente do ZASH
    int trustLevel = contextComponent->calculateTrust(req->context, req->user);
    return (float)trustLevel / 100.0;
}
```

#### Activity Score
```cpp
float calculateActivityScore(Request *req) {
    // Usa cadeia de Markov específica do usuário
    float activityProb = activityComponent->getUserActivityProbability(req);
    
    // Se ainda construindo, usa userLevel como fallback
    if (activityComponent->isMarkovBuilding) {
        return (float)req->user->userLevel->weight / 100.0;
    }
    
    return activityProb;
}
```

#### Context Score
```cpp
float calculateContextScore(Request *req) {
    // Combina fatores de contexto
    float accessWay = req->context->accessWay->weight / 100.0;
    float localization = req->context->localization->weight / 100.0;
    float time = req->context->time->weight / 100.0;
    float group = req->context->group->weight / 100.0;
    
    return (accessWay + localization + time + group) / 4.0;
}
```

## Exemplos de Uso

### 1. Configuração Básica

```cpp
// Criar componente de conflito
ConflictComponent *conflictComponent = new ConflictComponent(config, audit);

// Configurar interferências
conflictComponent->addDeviceInterference(1, 2, 1.0);  // AC + Aquecedor
conflictComponent->addDeviceInterference(3, 4, 0.8);  // TV + Som

// Configurar prioridades
conflictComponent->setUserPriority(1, 100);  // Admin
conflictComponent->setUserPriority(2, 80);   // Adulto

// Integrar com AuthorizationComponent
AuthorizationComponent *auth = new AuthorizationComponent(
    config, ontology, context, activity, conflictComponent, notification, audit
);
```

### 2. Processamento de Requisições

```cpp
// Processar requisição
Request *req = new Request(id, device, user, context, action, attackId, time);

// O ConflictComponent é chamado automaticamente pelo AuthorizationComponent
bool authorized = auth->authorizeRequest(req, explicitAuth);
```

### 3. Monitoramento de Conflitos

```cpp
// Verificar conflitos ativos
for (auto& pair : conflictComponent->activeConflicts) {
    Conflict *conflict = pair.second;
    cout << "Conflito ativo: " << *conflict << endl;
}

// Verificar dispositivos em uso
for (auto& pair : conflictComponent->deviceInUse) {
    int deviceId = pair.first;
    Request *req = pair.second;
    cout << "Dispositivo " << deviceId << " em uso por usuário " << req->user->id << endl;
}
```

## Cenários de Teste

### 1. Conflito Concorrente Simples

```cpp
// Usuário 1 tenta ligar TV
Request *req1 = new Request(1, tv, user1, context1, control, 0, time1);

// Usuário 2 tenta desligar TV (simultaneamente)
Request *req2 = new Request(2, tv, user2, context2, control, 0, time2);

// Apenas um será autorizado baseado na prioridade
```

### 2. Conflito Paralelo Complexo

```cpp
// Usuário 1 liga ar condicionado
Request *req1 = new Request(1, ac, user1, context1, control, 0, time1);

// Usuário 2 tenta ligar aquecedor (interferência total)
Request *req2 = new Request(2, heater, user2, context2, control, 0, time2);

// Sistema detecta interferência e resolve baseado na prioridade
```

### 3. Conflito com Múltiplos Dispositivos

```cpp
// Usuário 1 liga TV
Request *req1 = new Request(1, tv, user1, context1, control, 0, time1);

// Usuário 2 liga sistema de som (interfere com TV)
Request *req2 = new Request(2, sound, user2, context2, control, 0, time2);

// Usuário 3 liga micro-ondas (interfere com som)
Request *req3 = new Request(3, microwave, user3, context3, control, 0, time3);

// Sistema resolve conflitos em cadeia
```

## Logs e Auditoria

O módulo gera logs detalhados para auditoria:

```
CONFLICT LOG: Conflict[1,CONCURRENT,2 requests,2024-01-01 10:00:00]
  - Request 1 (User 1, Device 1, Action CONTROL)
  - Request 2 (User 2, Device 1, Action CONTROL)

CONFLICT RESOLUTION: Conflict 1 resolved. Winner: Request 1 (User 1)
```

## Configurações Recomendadas

### Para Casas Pequenas (1-3 usuários)
- `conflictTimeout = 15` segundos
- `maxConcurrentRequests = 3`
- Interferências simples entre dispositivos básicos

### Para Casas Médias (3-5 usuários)
- `conflictTimeout = 30` segundos
- `maxConcurrentRequests = 5`
- Interferências moderadas entre dispositivos

### Para Casas Grandes (5+ usuários)
- `conflictTimeout = 60` segundos
- `maxConcurrentRequests = 10`
- Interferências complexas entre múltiplos dispositivos

## Troubleshooting

### Problemas Comuns

1. **Conflitos não detectados**: Verificar se as interferências estão configuradas corretamente
2. **Resolução incorreta**: Verificar prioridades dos usuários
3. **Timeout de conflitos**: Ajustar `conflictTimeout` conforme necessário
4. **Dispositivos não liberados**: Verificar se `releaseDevice()` está sendo chamado

### Debug

```cpp
// Habilitar logs detalhados
conflictComponent->auditComponent->zashOutput = &cout;

// Verificar estado interno
conflictComponent->showActiveConflicts();
conflictComponent->showDeviceUsage();
```

## Contribuições

Para contribuir com o módulo:

1. Adicione novos tipos de conflitos se necessário
2. Implemente algoritmos de resolução mais sofisticados
3. Adicione métricas de performance
4. Melhore a configuração de interferências
5. Adicione testes unitários

## Referências

- Documentação do ZASH principal
- Algoritmos de resolução de conflitos
- Sistemas de priorização de usuários
- Modelos de interferência entre dispositivos

