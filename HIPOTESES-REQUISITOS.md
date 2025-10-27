# Hipóteses e Requisitos do Sistema ZASH

**Título:** Sistema de Resolução de Conflitos Multi-Métrica para Smart Homes  
**Data:** Outubro de 2025  
**Autor:** Pedro

---

## Objetivo Geral

Implementar e validar mecanismos avançados de resolução de conflitos em sistemas de casas inteligentes, utilizando abordagens hierárquicas e multi-métricas para garantir decisões justas e eficientes quando múltiplos usuários competem pelo controle de dispositivos.

---

## Hipóteses de Pesquisa

### **H1: Eficácia da Resolução Hierárquica**
> "O sistema de resolução hierárquica baseado em níveis de usuário (Admin > Adult > Child > Visitor) resolve pelo menos **90%** dos conflitos de forma adequada."

**Métrica:** Taxa de Adequação Hierárquica (TAH)  
**Validação:** TAH ≥ 90%

---

### **H2: Acurácia da Resolução Multi-Métrica**
> "O sistema multi-métrica (Trust × Activity) resolve pelo menos **85%** dos conflitos em empates hierárquicos de forma adequada."

**Métrica:** Acurácia Multi-Métrica (AMM)  
**Validação:** AMM ≥ 85%

---

### **H3: Performance do Sistema**
> "O tempo médio de resolução de conflitos é inferior a **100ms**, garantindo experiência responsiva."

**Métrica:** Tempo Médio de Resolução (TMR)  
**Validação:** TMR ≤ 100ms

---

### **H4: Detecção de Conflitos Concorrentes**
> "O sistema detecta conflitos concorrentes com precisão, utilizando timeouts específicos por tipo de dispositivo."

**Métricas:**
- Total de conflitos detectados
- Taxa de falsos positivos ≤ 15%

**Validação:** Comparação com ground truth dos logs

---

### **H5: Impacto da Cadeia de Markov**
> "O uso de cadeias de Markov (global vs. personalizada) impacta a diferenciação de usuários em conflitos multi-métrica."

**Métricas:**
- Distribuição de probabilidades (0.0, intermediárias, 1.0)
- Percentual de conflitos resolvidos por diferença em Activity Score

**Validação:** Comparação entre simulações com cadeia global e personalizada

---

### **H6: Escalabilidade**
> "O sistema mantém performance adequada (TMR ≤ 100ms) com até 10 usuários simultâneos."

**Métrica:** TMR vs. número de usuários  
**Validação:** Experimentos com 1, 5, 10 usuários

---

## Requisitos Funcionais

### **RF1: Detecção de Conflitos Concorrentes**
- **Descrição:** Sistema deve detectar quando múltiplos usuários tentam interagir com o mesmo dispositivo dentro de uma janela temporal (timeout)
- **Implementação:** `ConflictComponent::detectConflict()`
- **Validação:** Logs mostram "CONCURRENT CONFLICT detected"

---

### **RF2: Resolução Hierárquica**
- **Descrição:** Sistema deve resolver conflitos priorizando usuários com maior nível hierárquico
- **Ordem:** Admin > Adult > Child > Visitor
- **Implementação:** `ConflictComponent::selectWinnerMultiMetric()` - verificação de `user->userLevel->weight`
- **Validação:** TAH ≥ 90%

---

### **RF3: Resolução Multi-Métrica**
- **Descrição:** Em empates hierárquicos, sistema deve usar score combinado (Trust × Activity)
- **Fórmula:** `finalScore = trustScore × activityScore`
- **Implementação:** `ConflictComponent::calculateMultiMetricScore()`
- **Validação:** AMM ≥ 85%

---

### **RF4: Timeout Configurável por Dispositivo**
- **Descrição:** Cada tipo de dispositivo deve ter timeout específico baseado em características de uso
- **Exemplos:**
  - Luzes: 5s (ação rápida)
  - TV: 30s (ação moderada)
  - Ar-condicionado: 600s (ação lenta)
  - Fechaduras: 60s (crítico)
- **Implementação:** `Device->conflictTimeout`
- **Validação:** Logs mostram timeouts corretos

---

### **RF5: Cálculo de Tempo de Resolução**
- **Descrição:** Sistema deve medir e registrar tempo entre detecção e resolução
- **Implementação:** `conflict->detectionTime` e cálculo em microsegundos
- **Validação:** Métricas em `zash_simulation_metrics_*.txt`

---

### **RF6: Minimização de Falsos Conflitos**
- **Descrição:** Sistema não deve detectar conflito quando o mesmo usuário faz ações sequenciais
- **Implementação:** Verificação `lastReq->user->id == req->user->id` em `detectConflict()`
- **Validação:** Taxa de falsos conflitos ≤ 15%

---

### **RF7: Uso de Contexto na Resolução**
- **Descrição:** Score de Trust deve considerar contexto (local, horário, modo de acesso)
- **Implementação:** `ContextComponent::calculateTrust()`
- **Validação:** Logs mostram valores de Trust calculados

---

### **RF8: Uso de Atividade na Resolução**
- **Descrição:** Score de Activity deve usar Cadeia de Markov para avaliar normalidade do comportamento
- **Implementação:** `ActivityComponent` com `MarkovChain::getProbability()`
- **Validação:** Logs mostram probabilidades calculadas

---

## Requisitos Não-Funcionais

### **RNF1: Performance**
- **Descrição:** Sistema deve responder em tempo real
- **Especificação:** TMR ≤ 100ms
- **Justificativa:** Experiência do usuário não deve ser impactada

---

### **RNF2: Confiabilidade**
- **Descrição:** Sistema deve resolver 100% dos conflitos detectados
- **Especificação:** Taxa de sucesso = 100%
- **Validação:** `conflictsResolved / totalConflicts = 1.0`

---

### **RNF3: Usabilidade**
- **Descrição:** Decisões devem ser compreensíveis e auditáveis
- **Especificação:** Logs detalhados com justificativas
- **Validação:** Análise qualitativa dos logs

---

### **RNF4: Manutenibilidade**
- **Descrição:** Código deve ser modular e bem documentado
- **Especificação:** Separação clara entre componentes (Ontology, Context, Activity, Conflict)
- **Validação:** Revisão de código

---

### **RNF5: Flexibilidade**
- **Descrição:** Timeouts e thresholds devem ser configuráveis
- **Especificação:** Parâmetros ajustáveis sem recompilação
- **Validação:** Testes com diferentes configurações

---

## Cenários de Teste

### **Cenário 1: Conflito Hierárquico Simples**
- **Setup:** Admin e Adult tentam controlar mesmo dispositivo
- **Esperado:** Admin ganha automaticamente
- **Validação:** TAH

---

### **Cenário 2: Conflito Multi-Métrica (Adult vs Adult)**
- **Setup:** Dois Adults tentam controlar mesmo dispositivo
- **Esperado:** Vence quem tem maior score (Trust × Activity)
- **Validação:** AMM

---

### **Cenário 3: Conflito Multi-Métrica (Child vs Child)**
- **Setup:** Dois Children tentam controlar mesmo dispositivo
- **Esperado:** Vence quem tem maior score
- **Validação:** AMM

---

### **Cenário 4: Uso Sequencial Sem Conflito**
- **Setup:** Mesmo usuário faz múltiplas ações no mesmo dispositivo
- **Esperado:** Nenhum conflito detectado
- **Validação:** RF6

---

### **Cenário 5: Timeout Expirado**
- **Setup:** Dois usuários tentam controlar dispositivo com intervalo > timeout
- **Esperado:** Nenhum conflito detectado
- **Validação:** RF4

---

### **Cenário 6: Timeout Dentro do Limite**
- **Setup:** Dois usuários tentam controlar dispositivo com intervalo < timeout
- **Esperado:** Conflito detectado e resolvido
- **Validação:** RF1, RF4

---

### **Cenário 7: Dispositivo Crítico com Timeout Longo**
- **Setup:** Conflito em fechadura (timeout 60s)
- **Esperado:** Conflito detectado mesmo após 30s
- **Validação:** RF4

---

### **Cenário 8: Dispositivo Rápido com Timeout Curto**
- **Setup:** Conflito em luz (timeout 5s)
- **Esperado:** Conflito só detectado dentro de 5s
- **Validação:** RF4

---

### **Cenário 9: Trust Alto vs Trust Baixo**
- **Setup:** Adult com contexto suspeito vs Adult com contexto normal
- **Esperado:** Vence quem tem maior Trust
- **Validação:** RF7

---

### **Cenário 10: Activity Normal vs Activity Anômala**
- **Setup:** Adult com padrão normal vs Adult com padrão anômalo
- **Esperado:** Vence quem tem maior Activity
- **Validação:** RF8

---

### **Cenário 11: Cadeia Global vs Cadeia Personalizada**
- **Setup:** Executar simulação com ambas as configurações
- **Esperado:** Cadeia global gera mais variação em Activity
- **Validação:** H5

---

### **Cenário 12: Múltiplos Conflitos Simultâneos**
- **Setup:** 3+ usuários tentam controlar mesmo dispositivo
- **Esperado:** Sistema resolve escolhendo 1 vencedor
- **Validação:** RF2, RF3

---

### **Cenário 13: Conflito em Múltiplos Dispositivos**
- **Setup:** Conflitos em dispositivos diferentes simultaneamente
- **Esperado:** Cada conflito resolvido independentemente
- **Validação:** RF1, RF5

---

### **Cenário 14: Escalabilidade com 10 Usuários**
- **Setup:** 10 usuários gerando conflitos
- **Esperado:** TMR ≤ 100ms
- **Validação:** H6

---

## Métricas de Avaliação

### **M1: Taxa de Adequação Hierárquica (TAH)**
```
TAH = (Conflitos Hierárquicos Corretos / Total de Conflitos Hierárquicos) × 100%
Meta: ≥ 90%
```

---

### **M2: Acurácia Multi-Métrica (AMM)**
```
AMM = (Resoluções Multi-Métrica Adequadas / Total de Conflitos Multi-Métrica) × 100%
Meta: ≥ 85%
```

---

### **M3: Tempo Médio de Resolução (TMR)**
```
TMR = Σ(tempo_resolução_i) / total_conflitos
Meta: ≤ 100ms
```

---

### **M4: Distribuição de Conflitos por Dispositivo**
```
Para cada dispositivo: count(conflitos)
```

---

### **M5: Distribuição de Vitórias por Usuário**
```
Para cada usuário: count(vitórias em conflitos)
```

---

### **M6: Taxa de Detecção de Conflitos**
```
TDC = (Conflitos Detectados / Conflitos Reais) × 100%
Meta: 100%
```

---

### **M7: Taxa de Falsos Conflitos**
```
TFC = (Falsos Conflitos / Total de Detecções) × 100%
Meta: ≤ 15%
```

---

### **M8: Distribuição de Probabilidades (Activity)**
```
- % Probability = 0.0
- % Probability = 1.0
- % Probability intermediárias (0.0 < p < 1.0)
```

---

### **M9: Taxa de Uso de Proof**
```
TUP = (Requisições com Proof / Total de Requisições) × 100%
```

---

### **M10: Overhead de Tempo (Conflito vs Sem Conflito)**
```
Overhead = TMR_com_conflito - TMR_sem_conflito
```

---

### **M11: Taxa de Sucesso Geral**
```
TSG = (Requisições Autorizadas / Total de Requisições) × 100%
```

---

## Metodologia Experimental

### **Experimento 1: Validação de Hipóteses Básicas (H1-H4)**
- **Dataset:** `d6_2m_0tm.csv` (6 dispositivos, 2 meses)
- **Usuários:** 5 (1 Admin, 2 Adults, 2 Children)
- **Configuração:** Modo Hard (buildInterval=7 dias)
- **Métricas:** M1-M7
- **Duração:** ~10 minutos de simulação

---

### **Experimento 2: Impacto da Cadeia de Markov (H5)**
- **Variações:**
  - Simulação A: Cadeias personalizadas por usuário
  - Simulação B: Cadeia global única
- **Comparação:** M8 (distribuição de probabilidades)
- **Hipótese:** Cadeia global → mais variação → melhor diferenciação

---

### **Experimento 3: Sensibilidade de Timeouts**
- **Variações:** Timeouts de 1s, 5s, 10s, 30s, 60s, 300s
- **Métricas:** M6, M7 (detecção e falsos positivos)
- **Análise:** Timeout ótimo para cada tipo de dispositivo

---

### **Experimento 4: Escalabilidade (H6)**
- **Variações:** 1, 3, 5, 10 usuários
- **Métrica:** M3 (TMR) vs. número de usuários
- **Análise:** Limite de escalabilidade do sistema

---

### **Experimento 5: Análise de Trust**
- **Foco:** Impacto de contexto nas resoluções
- **Cenários:**
  - Acesso interno vs externo
  - Horário comum vs incomum
  - Adulto vs criança
- **Métrica:** Correlação entre Trust e vitórias

---

### **Experimento 6: Análise de Activity**
- **Foco:** Impacto de padrões comportamentais
- **Cenários:**
  - Comportamento normal (prob = 1.0)
  - Comportamento anômalo (prob = 0.0)
  - Comportamento intermediário
- **Métrica:** Correlação entre Activity e vitórias

---

### **Experimento 7: Comparação com Baseline**
- **Baseline:** Sistema sem resolução de conflitos (FIFO)
- **ZASH:** Sistema completo multi-métrica
- **Comparação:**
  - Justiça nas decisões
  - Satisfação dos usuários (simulada)
  - Overhead de tempo

---

## Checklist de Validação

- [ ] H1 validada (TAH ≥ 90%)
- [ ] H2 validada (AMM ≥ 85%)
- [ ] H3 validada (TMR ≤ 100ms)
- [ ] H4 validada (detecção precisa)
- [ ] H5 testada (comparação cadeias)
- [ ] H6 testada (escalabilidade)
- [ ] Todos os RF1-RF8 implementados
- [ ] Todos os RNF1-RNF5 verificados
- [ ] Todos os 14 cenários testados
- [ ] Todas as 11 métricas coletadas
- [ ] 7 experimentos executados
- [ ] Dados analisados e plotados
- [ ] Documento com resultados completo
- [ ] Apresentação preparada

---