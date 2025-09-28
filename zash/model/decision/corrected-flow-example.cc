/**
 * Exemplo do Fluxo Corrigido de Autorização com Conflitos
 * 
 * Este exemplo demonstra o fluxo correto onde:
 * 1. Ontologia atua como FILTRO (não como peso)
 * 2. Conflitos são resolvidos apenas entre usuários JÁ AUTORIZADOS
 * 3. Multi-métrica usa apenas 3 métricas (Confiança, Atividade, Contexto)
 */

#include "ns3/zash-authorization.h"
#include "ns3/zash-conflict.h"
#include "ns3/zash-ontology.h"
#include "ns3/zash-context.h"
#include "ns3/zash-activity.h"

using namespace ns3;

void demonstrateCorrectedFlow() {
    cout << "=== DEMONSTRAÇÃO DO FLUXO CORRIGIDO ===" << endl;
    
    // Configuração do sistema
    ConfigurationComponent *config = new ConfigurationComponent();
    AuditComponent *audit = new AuditComponent();
    
    // Componentes de verificação
    OntologyComponent *ontology = new OntologyComponent(config, audit);
    ContextComponent *context = new ContextComponent(config, audit);
    ActivityComponent *activity = new ActivityComponent(config, audit);
    
    // Componente de conflitos (agora com dependências corretas)
    ConflictComponent *conflict = new ConflictComponent(config, audit);
    conflict->setOntologyComponent(ontology);
    conflict->setContextComponent(context);
    conflict->setActivityComponent(activity);
    
    // Componente de autorização (fluxo corrigido)
    NotificationComponent *notification = new NotificationComponent(config, audit);
    AuthorizationComponent *auth = new AuthorizationComponent(
        config, ontology, context, activity, conflict, notification, audit
    );
    
    cout << "\n=== CENÁRIO: Dois usuários tentam controlar TV ===" << endl;
    
    // Criar usuários
    User *admin = new User(1, "Admin", new UserLevel(1, "ADMIN", 100));
    User *child = new User(2, "Child", new UserLevel(3, "CHILD", 20));
    
    // Criar dispositivo e ação
    Device *tv = new Device(1, "TV", new DeviceClass(1, "ENTERTAINMENT"));
    Action *control = new Action(1, "CONTROL", 80);
    
    // Criar contextos
    Context *context1 = new Context(1, "HOME", "INTERNAL", 0, 0, 0, 0);
    Context *context2 = new Context(2, "HOME", "INTERNAL", 0, 0, 0, 0);
    
    // Criar requisições
    time_t now = time(nullptr);
    Request *req1 = new Request(1, tv, admin, context1, control, 0, now);
    Request *req2 = new Request(2, tv, child, context2, control, 0, now);
    
    cout << "\n--- REQUISIÇÃO 1: Admin tenta controlar TV ---" << endl;
    bool auth1 = auth->authorizeRequest(req1, [](Request*){ return true; });
    cout << "Resultado: " << (auth1 ? "AUTORIZADO" : "NEGADO") << endl;
    
    cout << "\n--- REQUISIÇÃO 2: Child tenta controlar TV ---" << endl;
    bool auth2 = auth->authorizeRequest(req2, [](Request*){ return true; });
    cout << "Resultado: " << (auth2 ? "AUTORIZADO" : "NEGADO") << endl;
    
    cout << "\n=== ANÁLISE DO FLUXO ===" << endl;
    cout << "1. Ontologia: Admin ✅ | Child ❌ (sem permissão)" << endl;
    cout << "2. Contexto: Admin ✅ | Child (não chega aqui)" << endl;
    cout << "3. Atividade: Admin ✅ | Child (não chega aqui)" << endl;
    cout << "4. Conflito: Admin ✅ | Child (não chega aqui)" << endl;
    
    cout << "\n=== VANTAGENS DO FLUXO CORRIGIDO ===" << endl;
    cout << "✅ Eficiência: Não processa conflitos para usuários sem permissão" << endl;
    cout << "✅ Lógica: Ontologia atua como filtro, não como peso" << endl;
    cout << "✅ Segurança: Apenas usuários autorizados competem" << endl;
    cout << "✅ Clareza: Cada etapa tem responsabilidade específica" << endl;
    
    cout << "\n=== CENÁRIO: Dois admins tentam controlar TV ===" << endl;
    
    // Criar segundo admin
    User *admin2 = new User(3, "Admin2", new UserLevel(1, "ADMIN", 100));
    Request *req3 = new Request(3, tv, admin2, context1, control, 0, now);
    
    cout << "\n--- REQUISIÇÃO 3: Admin2 tenta controlar TV ---" << endl;
    bool auth3 = auth->authorizeRequest(req3, [](Request*){ return true; });
    cout << "Resultado: " << (auth3 ? "AUTORIZADO" : "NEGADO") << endl;
    
    cout << "\n=== ANÁLISE DO FLUXO COM CONFLITO ===" << endl;
    cout << "1. Ontologia: Admin ✅ | Admin2 ✅ (ambos têm permissão)" << endl;
    cout << "2. Contexto: Admin ✅ | Admin2 ✅ (ambos passam)" << endl;
    cout << "3. Atividade: Admin ✅ | Admin2 ✅ (ambos passam)" << endl;
    cout << "4. Conflito: Resolve entre usuários JÁ AUTORIZADOS" << endl;
    cout << "   - Multi-métrica: Confiança(30%) + Atividade(40%) + Contexto(30%)" << endl;
    cout << "   - Vencedor: Baseado em pontuação combinada" << endl;
    
    // Limpeza
    delete config;
    delete audit;
    delete ontology;
    delete context;
    delete activity;
    delete conflict;
    delete notification;
    delete auth;
    delete admin;
    delete child;
    delete admin2;
    delete tv;
    delete control;
    delete context1;
    delete context2;
    delete req1;
    delete req2;
    delete req3;
}

int main() {
    demonstrateCorrectedFlow();
    return 0;
}
