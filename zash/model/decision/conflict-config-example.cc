/*
 * Exemplo de configuração de interferências entre dispositivos
 * Este arquivo demonstra como configurar o ConflictComponent
 * com diferentes tipos de interferências entre dispositivos
 */

#include "zash-conflict.h"

namespace ns3 {

void configureDeviceInterferences(ConflictComponent *conflictComponent) {
    /*
     * Configurações de interferência baseadas em cenários reais:
     * 
     * 1. Interferências de Energia:
     *    - Ar condicionado + Aquecedor (interferência total)
     *    - Múltiplas luzes (interferência baixa)
     * 
     * 2. Interferências de Segurança:
     *    - Sistema de alarme + Câmeras (interferência média)
     *    - Porta principal + Sistema de segurança (interferência alta)
     * 
     * 3. Interferências de Conforto:
     *    - TV + Sistema de som (interferência alta)
     *    - Ar condicionado + Ventilador (interferência média)
     * 
     * 4. Interferências de Funcionalidade:
     *    - Micro-ondas + WiFi (interferência baixa)
     *    - Aspirador + Sistema de som (interferência média)
     */
    
    // Interferências de Energia
    conflictComponent->addDeviceInterference(1, 2, 1.0);  // Ar condicionado + Aquecedor
    conflictComponent->addDeviceInterference(3, 4, 0.3);  // Luz sala + Luz cozinha
    conflictComponent->addDeviceInterference(3, 5, 0.3);  // Luz sala + Luz quarto
    
    // Interferências de Segurança
    conflictComponent->addDeviceInterference(6, 7, 0.7);  // Alarme + Câmeras
    conflictComponent->addDeviceInterference(8, 6, 0.9);  // Porta principal + Alarme
    
    // Interferências de Conforto
    conflictComponent->addDeviceInterference(9, 10, 0.8); // TV + Sistema de som
    conflictComponent->addDeviceInterference(1, 11, 0.6); // Ar condicionado + Ventilador
    
    // Interferências de Funcionalidade
    conflictComponent->addDeviceInterference(12, 13, 0.4); // Micro-ondas + WiFi
    conflictComponent->addDeviceInterference(14, 10, 0.5); // Aspirador + Sistema de som
}

void configureUserPriorities(ConflictComponent *conflictComponent) {
    /*
     * Configuração de prioridades dos usuários:
     * 
     * Prioridades baseadas em:
     * 1. Nível de usuário (ADMIN > ADULT > CHILD > VISITOR)
     * 2. Responsabilidade pela casa
     * 3. Frequência de uso
     * 4. Urgência da ação
     */
    
    // Prioridades padrão baseadas no userLevel
    // (já configuradas no construtor)
    
    // Ajustes específicos para cenários especiais
    conflictComponent->setUserPriority(1, 100);  // Admin principal
    conflictComponent->setUserPriority(2, 80);   // Adulto responsável
    conflictComponent->setUserPriority(3, 60);   // Adulto secundário
    conflictComponent->setUserPriority(4, 40);   // Criança
    conflictComponent->setUserPriority(5, 20);   // Visitante
}

void demonstrateConflictScenarios() {
    /*
     * Cenários de demonstração de conflitos:
     * 
     * 1. Conflito Concorrente:
     *    - Usuário 1 tenta ligar TV
     *    - Usuário 2 tenta desligar TV simultaneamente
     *    - Resolução: Prioridade do usuário
     * 
     * 2. Conflito Paralelo:
     *    - Usuário 1 liga ar condicionado
     *    - Usuário 2 tenta ligar aquecedor
     *    - Resolução: Interferência total detectada
     * 
     * 3. Conflito Complexo:
     *    - Múltiplos usuários em dispositivos interconectados
     *    - Resolução: Algoritmo de prioridade + interferência
     */
    
    // Exemplo de uso com resolução multi-métrica:
    /*
    // Criar componentes necessários
    OntologyComponent *ontology = new OntologyComponent(config, audit);
    ContextComponent *context = new ContextComponent(config, audit);
    ActivityComponent *activity = new ActivityComponent(data, config, audit);
    
    // Criar ConflictComponent com todos os componentes
    ConflictComponent *conflictComponent = new ConflictComponent(
        config, ontology, context, activity, audit
    );
    
    configureDeviceInterferences(conflictComponent);
    configureUserPriorities(conflictComponent);
    
    // Simular conflito
    Request *req1 = new Request(1, device1, user1, context1, action1, 0, time1);
    Request *req2 = new Request(2, device1, user2, context2, action2, 0, time2);
    
    bool result1 = conflictComponent->processRequest(req1);
    bool result2 = conflictComponent->processRequest(req2);
    
    // Apenas um dos requests será autorizado baseado na pontuação multi-métrica
    // que considera: ontologia, confiança, atividade do usuário e contexto
    */
}

} // namespace ns3

