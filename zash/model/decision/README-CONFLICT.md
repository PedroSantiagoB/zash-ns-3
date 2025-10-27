# Módulo ZASH-Conflict

## Visão Geral

O módulo `zash-conflict` é uma extensão do sistema ZASH que lida com conflitos entre usuários em casas inteligentes. Ele detecta e resolve conflitos concorrentes usando uma abordagem **multi-métrica inspirada em KRATOS**:

1. **Conflitos Concorrentes**: Múltiplos usuários tentando interagir com o mesmo dispositivo simultaneamente

## Abordagem Hierárquica Simplificada

O sistema utiliza uma **abordagem hierárquica simplificada** para resolução de conflitos:

1. **Hierarquia por User-Level**: Usuários com nível mais alto ganham automaticamente
2. **Score Simplificado**: Apenas para empates no mesmo nível (Confiança × Atividade)

**Lógica de Resolução:**
- **Admin vs Adulto**: Admin ganha automaticamente
- **Adulto vs Criança**: Adulto ganha automaticamente  
- **Adulto vs Adulto**: Usa score (Confiança × Atividade)
- **Criança vs Criança**: Usa score (Confiança × Atividade)

**Nota**: A ontologia atua como **filtro** (não como peso), garantindo que apenas usuários com permissão participem da resolução de conflitos.

## Arquitetura

### Componentes Principais

- **ConflictComponent**: Componente principal que gerencia conflitos
- **Conflict**: Estrutura que representa um conflito específico
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

### Conflitos Concorrentes

**Definição**: Dois ou mais usuários tentam interagir com o mesmo dispositivo dentro de um período de tempo configurável por dispositivo.

**Lógica de Timeout:**
Cada dispositivo tem seu próprio timeout baseado em suas características:
- **Dispositivos Rápidos** (Luz): 5-10s (ações instantâneas)
- **Dispositivos Moderados** (TV): 10-30s (ações rápidas)
- **Dispositivos Lentos** (AC): 300-600s (ações demoradas)
- **Dispositivos Críticos** (Segurança): 60-300s (ações importantes)

**Exemplo com TV (timeout=10s)**:
```
Usuário A: Liga TV (t=0s) → Autorizado, timestamp registrado
Usuário B: Desliga TV (t=5s) → Conflito! (5s < 10s timeout)
Usuário C: Liga TV (t=15s) → Sem conflito (15s > 10s timeout)
```

**Exemplo com AC (timeout=600s)**:
```
Usuário A: Liga AC (t=0s) → Autorizado, timestamp registrado
Usuário B: Desliga AC (t=300s) → Conflito! (300s < 600s timeout)
Usuário C: Liga AC (t=700s) → Sem conflito (700s > 600s timeout)
```

**Resolução**: Baseada em:
1. **Hierarquia**: User-level mais alto ganha automaticamente
2. **Score**: Para mesmo user-level, usa Confiança × Atividade

### Cadeias de Markov por Usuário com Fallback

O sistema utiliza **cadeias de Markov individuais** para cada usuário, permitindo modelagem personalizada de comportamento. Para garantir funcionamento adequado com usuários novos, implementamos um **sistema de fallback**:

**Lógica:**
1. **Cadeia Individual**: Cada usuário possui sua própria cadeia de Markov
2. **Fallback Global**: Se cadeia do usuário está vazia, usa cadeia global da casa
3. **Construção Contínua**: Cadeias individuais são construídas a cada requisição

**Cenários:**

**Usuário Existente (cadeia populada):**
```
Usuário A: 30 dias de histórico
├─ ActivityComponent.verifyActivity()
│  └─ Usa cadeia pessoal de A (prob baseada em seu comportamento)
└─ ConflictComponent.calculateActivityScore()
   └─ Usa cadeia pessoal de A (prob baseada em seu comportamento)
```

**Usuário Novo (cadeia vazia):**
```
Usuário B: Primeiro dia no sistema
├─ ActivityComponent.verifyActivity()
│  ├─ Cadeia de B está vazia
│  └─ Fallback: usa cadeia global da casa (prob baseada no padrão geral)
└─ ConflictComponent.calculateActivityScore()
   ├─ Cadeia de B está vazia
   └─ Fallback: usa cadeia global da casa (prob baseada no padrão geral)
```

**Usuário em Transição (cadeia parcial):**
```
Usuário C: 5 dias de histórico
├─ Transição já vista por C → usa cadeia pessoal
└─ Transição nunca vista por C → fallback para cadeia global
```

**Vantagens:**
- **Usuários novos** não são penalizados por falta de histórico
- **Modelagem gradual** de comportamento individual
- **Padrão da casa** serve como baseline para todos
- **Não requer** período de construção individual

**Implementação:**
```cpp
// Em verifyActivity e getUserActivityProbability
MarkovChain *userMarkov = getUserMarkovChain(req->user->id);
float prob = userMarkov->getProbability(currentState, lastState);

// Se cadeia do usuário está vazia, usar cadeia global
if (prob == 0.0 && userMarkov->transitionMatrix.empty()) {
    prob = markovChain->getProbability(currentState, lastState);
}
```

## Configuração

### 1. Prioridades dos Usuários

```cpp
// Configurar prioridade de usuário
conflictComponent->setUserPriority(userId, priority);

// Exemplos:
conflictComponent->setUserPriority(1, 100);  // Admin
conflictComponent->setUserPriority(2, 80);   // Adulto
conflictComponent->setUserPriority(3, 40);   // Criança
```

### 2. Parâmetros do Sistema

```cpp
// Configurar timeout geral de conflitos
conflictComponent->conflictTimeout = 30;  // 30 segundos

// Configurar timeout específico por dispositivo
devices[0]->conflictTimeout = 10;   // TV: 10 segundos
devices[1]->conflictTimeout = 600;  // Ar Condicionado: 600 segundos (10 minutos)
devices[2]->conflictTimeout = 300;  // Sistema de Segurança: 300 segundos (5 minutos)
devices[3]->conflictTimeout = 5;    // Luz: 5 segundos
```

**Parâmetros de Timeout:**
- `conflictTimeout`: Timeout geral para resolução de conflitos (30s)
- `device->conflictTimeout`: Timeout específico de cada dispositivo (padrão 10s)

**Exemplos de Timeout por Tipo de Dispositivo:**
- **Dispositivos Rápidos** (Luz, Cortina): 5-10 segundos
- **Dispositivos Moderados** (TV, Som): 10-30 segundos  
- **Dispositivos Lentos** (AC, Aquecedor): 300-600 segundos
- **Dispositivos Críticos** (Segurança, Portas): 60-300 segundos

## Algoritmos de Resolução Hierárquica

### 1. Lógica de Resolução de Conflitos

```cpp
Request* selectWinnerMultiMetric(vector<Request*> requests) {
    // 1. Verificar hierarquia por user-level
    Request *highestLevelReq = nullptr;
    int maxUserLevel = -1;
    
    for (Request *req : requests) {
        int userLevel = req->user->userLevel->weight;
        if (userLevel > maxUserLevel) {
            maxUserLevel = userLevel;
            highestLevelReq = req;
        }
    }
    
    // 2. Verificar se há empate no nível mais alto
    vector<Request*> sameLevelRequests;
    for (Request *req : requests) {
        if (req->user->userLevel->weight == maxUserLevel) {
            sameLevelRequests.push_back(req);
        }
    }
    
    if (sameLevelRequests.size() == 1) {
        // Apenas um usuário com nível mais alto - ganha automaticamente
        return highestLevelReq;
    }
    
    // 3. Empate no nível mais alto - usar score (Confiança × Atividade)
    Request *winner = nullptr;
    float maxScore = -1.0;
    
    for (Request *req : sameLevelRequests) {
        float trustScore = calculateTrustScore(req);
        float activityScore = calculateActivityScore(req);
        float finalScore = trustScore * activityScore;
        
        if (finalScore > maxScore) {
            maxScore = finalScore;
            winner = req;
        }
    }
    
    return winner;
}
```

### 2. Cálculo de Score Simplificado

```cpp
float calculateSimplifiedScore(Request *req) {
    // Confiança do contexto
    float trustScore = contextComponent->calculateTrust(req->context, req->user) / 100.0;
    
    // Atividade da cadeia de Markov
    float activityScore = activityComponent->getUserActivityProbability(req);
    
    // Score final = Confiança × Atividade
    return trustScore * activityScore;
}
```

### 3. Métricas Individuais

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

## Exemplos de Uso

### 1. Configuração Básica

```cpp
// Criar componente de conflito
ConflictComponent *conflictComponent = new ConflictComponent(config, ontology, context, activity, audit);

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

### 1. Conflito Hierárquico (Admin vs Adulto)

```cpp
// Usuário Admin (level=100) tenta ligar TV (t=0s)
Request *req1 = new Request(1, tv, admin, context1, control, 0, time1);
// Requisição autorizada, timestamp registrado

// Usuário Adulto (level=80) tenta desligar TV (t=5s) - dentro do timeout
Request *req2 = new Request(2, tv, adult, context2, control, 0, time2);
// Conflito detectado! Admin ganha automaticamente por hierarquia
```

### 2. Conflito por Score (Adulto vs Adulto)

```cpp
// Adulto A (level=80) tenta ligar TV (t=0s)
Request *req1 = new Request(1, tv, adultA, context1, control, 0, time1);
// Autorizado, timestamp registrado

// Adulto B (level=80) tenta desligar TV (t=3s)
Request *req2 = new Request(2, tv, adultB, context2, control, 0, time2);
// Conflito detectado! Mesmo user-level - usa score (Confiança × Atividade)

// Adulto C (level=80) tenta mudar canal (t=12s)
Request *req3 = new Request(3, tv, adultC, context3, control, 0, time3);
// Sem conflito (timeout expirado), autorizado

// Sistema resolve conflito entre Adulto A e B usando score simplificado
```

## Logs e Auditoria

O módulo gera logs detalhados para auditoria:

```
=== SIMPLIFIED CONFLICT RESOLUTION ===
User 1 has level: 100
User 2 has level: 80
Winner by user-level hierarchy: User 1 (level 100)

CONFLICT LOG: Conflict[1,CONCURRENT,2 requests,2024-01-01 10:00:00]
  - Request 1 (User 1, Device 1, Action CONTROL)
  - Request 2 (User 2, Device 1, Action CONTROL)

CONFLICT RESOLUTION: Conflict 1 resolved. Winner: Request 1 (User 1)
```

## Configurações Recomendadas

### Timeouts Recomendados por Tipo de Dispositivo

**Dispositivos Rápidos:**
- Luz: `5-10s`
- Cortina: `5-10s`
- Tomada: `5-10s`

**Dispositivos Moderados:**
- TV: `10-30s`
- Som: `10-30s`
- Ventilador: `30-60s`

**Dispositivos Lentos:**
- Ar Condicionado: `300-600s` (5-10 minutos)
- Aquecedor: `300-600s` (5-10 minutos)
- Geladeira: `600-1200s` (10-20 minutos)

**Dispositivos Críticos:**
- Segurança: `60-300s` (1-5 minutos)
- Portas/Fechaduras: `60-180s` (1-3 minutos)
- Câmeras: `120-300s` (2-5 minutos)

## Troubleshooting

### Problemas Comuns

1. **Conflitos não detectados**: Verificar se `deviceInUse` está sendo atualizado corretamente
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

1. Implemente algoritmos de resolução mais sofisticados
2. Adicione métricas de performance
3. Melhore a sincronização de `deviceInUse` com estado real dos dispositivos
4. Adicione testes unitários

## Referências

- Documentação do ZASH principal
- Algoritmos de resolução de conflitos
- Sistemas de priorização de usuários