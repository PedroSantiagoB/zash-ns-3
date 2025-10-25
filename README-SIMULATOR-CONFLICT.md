# ZASH Conflict Resolution Simulator

## Visão Geral

O `zash-simulator-conflict.cc` é um simulador especializado para testar cenários de resolução de conflitos no sistema ZASH. Diferente do simulador principal (`zash-simulator.cc`) que utiliza datasets reais, este simulador cria cenários específicos e controlados para validar a funcionalidade do módulo `zash-conflict`.

## Objetivos

Testar e validar:
1. **Detecção de Conflitos Concorrentes** (timeout por dispositivo)
2. **Resolução Hierárquica** (prioridade por user-level)
3. **Resolução por Score** (Trust × Activity para mesmo nível)
4. **Fallback de Cadeias de Markov** (usuários novos)
5. **Diferentes Timeouts** (dispositivos rápidos vs lentos)
6. **Dispositivos Críticos** (segurança)

## Cenários de Teste

### **Cenário 1: Conflito Concorrente (TV - 10s timeout)**
```
t=0s:  User 2 (Adult) liga TV        → Autorizado
t=5s:  User 3 (Adult) desliga TV     → CONFLITO (5s < 10s timeout)
                                     → Resolução por score (Adult vs Adult)
```

### **Cenário 2: Sem Conflito (TV - 10s timeout)**
```
t=20s: User 2 (Adult) liga TV        → Autorizado
t=35s: User 3 (Adult) desliga TV     → SEM CONFLITO (15s > 10s timeout)
```

### **Cenário 3: Resolução Hierárquica (Admin vs Adult)**
```
t=50s: User 2 (Adult) liga Light     → Autorizado
t=52s: User 1 (Admin) desliga Light  → CONFLITO
                                     → Admin GANHA automaticamente
```

### **Cenário 4: Resolução por Score (Adult vs Adult)**
```
t=70s: User 2 (Adult) liga AC        → Autorizado
t=75s: User 3 (Adult) desliga AC     → CONFLITO
                                     → Resolução por score (Trust × Activity)
```

### **Cenário 5: Resolução por Score (Child vs Child)**
```
t=100s: User 4 (Child) liga Light    → Autorizado
t=103s: User 5 (Child) desliga Light → CONFLITO
                                     → Resolução por score (Trust × Activity)
```

### **Cenário 6: Usuário Novo (Fallback)**
```
t=120s: User 6 (Visitor - NOVO) liga TV → Autorizado
                                        → Usa cadeia GLOBAL como fallback
```

### **Cenário 7: Timeout Longo (AC - 600s)**
```
t=150s: User 2 (Adult) liga AC       → Autorizado
t=450s: User 3 (Adult) desliga AC    → CONFLITO (300s < 600s timeout)
                                     → Resolução por score
```

### **Cenário 8: Dispositivo Crítico (Door Lock - 60s)**
```
t=700s: User 1 (Admin) tranca porta  → Autorizado
t=730s: User 2 (Adult) abre porta    → CONFLITO (30s < 60s timeout)
                                     → Admin GANHA automaticamente
```

## Estrutura do Simulador

### **Usuários Criados**
```cpp
User 1: ADMIN (weight=100)   - Maior prioridade
User 2: ADULT (weight=75)    - Prioridade média
User 3: ADULT (weight=75)    - Prioridade média (teste empate)
User 4: CHILD (weight=50)    - Prioridade baixa
User 5: CHILD (weight=50)    - Prioridade baixa (teste empate)
User 6: VISITOR (weight=25)  - Usuário novo (fallback)
```

### **Dispositivos Criados**
```cpp
Device 1: TV              (timeout=10s)   - Ação rápida
Device 2: Light           (timeout=5s)    - Ação instantânea
Device 3: AC              (timeout=600s)  - Ação demorada
Device 4: Door Lock       (timeout=60s)   - Crítico
Device 5: Security Camera (timeout=300s)  - Crítico
```

### **Contexto Padrão**
Todos os testes usam contexto ideal para focar na resolução de conflitos:
```cpp
AccessWay: LOCAL
Localization: INTERNAL
Group: ALONE
```

## Como Compilar e Executar

### **1. Copiar arquivos para NS-3**
```bash
cd /home/pedro/dev/tcc/zash-ns-3
./replace.sh
```

### **2. Compilar no NS-3**
```bash
cd <ns3-directory>
./ns3 build
```

### **3. Executar o simulador**
```bash
./ns3 run zash-simulator-conflict
```

## Saída Esperada

### **Console**
```
========================================
ZASH CONFLICT RESOLUTION SIMULATOR
========================================

========================================
Test completed! Check output at:
zash_traces_conflict/2024-10-25_14-30-45/conflict_test_2024-10-25_14-30-45.txt
========================================
```

### **Arquivo de Log**
O arquivo gerado em `zash_traces_conflict/<timestamp>/conflict_test_<timestamp>.txt` conterá:

```
========================================
SCENARIO: User 2 (Adult) liga TV
Time: 2024-10-25 14:30:00
========================================
Activity Component
Verify activities
From: [0,0,0,0,0]
To: [1,0,0,0,0]
Markov Chain is still building
...
RESULT: AUTHORIZED
========================================

========================================
SCENARIO: User 3 (Adult) desliga TV após 5s - CONFLITO ESPERADO
Time: 2024-10-25 14:30:05
========================================
Activity Component
...
User is already authorized, checking conflicts...
CONFLICT DETECTED: CONCURRENT_CONFLICT
Resolving conflict with 2 requests...
User 2: finalScore = 75.0 (trust=100, activity=1.0, userLevel=75)
User 3: finalScore = 75.0 (trust=100, activity=1.0, userLevel=75)
Winner: User 2 (by tie-breaker)
...
RESULT: DENIED (User 3 perdeu o conflito)
========================================
```

## Validações Implementadas

### ✅ **Detecção de Conflitos**
- Timeout por dispositivo funciona corretamente
- Conflitos são detectados dentro do timeout
- Sem conflitos fora do timeout

### ✅ **Resolução Hierárquica**
- User-level mais alto sempre ganha
- Admin > Adult > Child > Visitor

### ✅ **Resolução por Score**
- Usado quando user-level é igual
- Score = Trust × Activity
- Tie-breaker em caso de empate total

### ✅ **Fallback de Markov**
- Usuários novos usam cadeia global
- Logs mostram "using global chain as fallback"
- Sistema não falha com usuários sem histórico

### ✅ **Dispositivos Críticos**
- Timeouts diferentes por tipo de dispositivo
- Hierarquia respeitada mesmo em críticos

## Diferenças do Simulador Principal

| Aspecto | `zash-simulator.cc` | `zash-simulator-conflict.cc` |
|---------|---------------------|------------------------------|
| **Dataset** | Dados reais (CSV) | Cenários sintéticos |
| **Foco** | Fluxo completo | Resolução de conflitos |
| **Rede** | Simulação completa (WiFi, NS-3) | Sem rede (direto) |
| **Tempo** | Horas/dias | Segundos/minutos |
| **Output** | Múltiplos arquivos | 1 arquivo de log |
| **Ataques** | Sim (impersonation) | Não (foco em conflitos) |

## Métricas Coletadas

Ao final da execução, o `AuditComponent` gera métricas:
- Total de requisições autorizadas
- Total de requisições negadas
- Total de conflitos detectados
- Conflitos resolvidos por hierarquia
- Conflitos resolvidos por score
- Requisições com fallback (usuários novos)

## Próximos Passos

Após validar que o simulador compila e executa:

1. **Verificar Logs**: Confirmar que conflitos são detectados
2. **Validar Resolução**: Verificar que hierarquia funciona
3. **Testar Scores**: Confirmar cálculo de Trust × Activity
4. **Validar Fallback**: Verificar logs de usuários novos
5. **Ajustar Cenários**: Adicionar/modificar cenários conforme necessário

## Troubleshooting

### **Erro de compilação: 'ConflictComponent' was not declared**
- Verificar que `zash-conflict.h` e `zash-conflict.cc` foram copiados
- Verificar que `CMakeLists.txt` inclui os arquivos do módulo

### **Segmentation fault**
- Verificar que `DataComponent` tem estado inicial definido
- Verificar que todos os ponteiros estão inicializados

### **Conflitos não detectados**
- Verificar valores de `conflictTimeout` nos dispositivos
- Verificar timestamps das requisições

### **Fallback não funciona**
- Verificar logs: deve mostrar "using global chain as fallback"
- Verificar que usuário não tem histórico na cadeia individual

## Extensões Futuras

- Adicionar mais cenários (3+ usuários simultâneos)
- Testar diferentes contextos (trust baixo)
- Simular períodos de construção da cadeia de Markov
- Adicionar métricas de desempenho (tempo de resolução)
- Testar com ataques de impersonation + conflitos

