/*
 * Exemplo prático da resolução multi-métrica de conflitos
 * Demonstra como o sistema KRATOS-inspired resolve conflitos
 * usando ontologia, confiança, atividade e contexto
 */

#include "zash-conflict.h"

namespace ns3 {

void demonstrateMultiMetricResolution() {
    /*
     * Cenário: Dois usuários tentam controlar a TV simultaneamente
     * 
     * Usuário 1: ADMIN, dentro de casa, horário comum, sozinho
     * Usuário 2: CHILD, fora de casa, horário incomum, com outros
     * 
     * Vamos calcular as pontuações multi-métricas para cada usuário
     */
    
    // Simulação de dados (em um sistema real, estes viriam das requisições)
    /*
    User *admin = new User(1, ADMIN_LEVEL, ADULT_AGE);
    User *child = new User(2, CHILD_LEVEL, KID_AGE);
    
    Device *tv = new Device(1, "TV", NONCRITICAL_CLASS, 1, true);
    
    Context *adminContext = new Context(REQUESTED, INTERNAL, COMMON, TOGETHER);
    Context *childContext = new Context(PERSONAL, EXTERNAL, UNCOMMON, ALONE);
    
    Request *adminReq = new Request(1, tv, admin, adminContext, CONTROL, 0, time1);
    Request *childReq = new Request(2, tv, child, childContext, CONTROL, 0, time2);
    */
    
    cout << "=== EXEMPLO DE RESOLUÇÃO MULTI-MÉTRICA ===" << endl;
    cout << endl;
    
    cout << "CENÁRIO: Dois usuários tentam controlar a TV simultaneamente" << endl;
    cout << endl;
    
    cout << "USUÁRIO 1 (ADMIN):" << endl;
    cout << "  - Nível: ADMIN (weight: 70)" << endl;
    cout << "  - Contexto: REQUESTED + INTERNAL + COMMON + TOGETHER" << endl;
    cout << "  - Atividade: Padrão normal (probabilidade alta)" << endl;
    cout << "  - Ontologia: Permite CONTROL em NONCRITICAL" << endl;
    cout << endl;
    
    cout << "USUÁRIO 2 (CHILD):" << endl;
    cout << "  - Nível: CHILD (weight: 30)" << endl;
    cout << "  - Contexto: PERSONAL + EXTERNAL + UNCOMMON + ALONE" << endl;
    cout << "  - Atividade: Padrão anômalo (probabilidade baixa)" << endl;
    cout << "  - Ontologia: Permite CONTROL em NONCRITICAL" << endl;
    cout << endl;
    
    cout << "CÁLCULO DAS PONTUAÇÕES:" << endl;
    cout << endl;
    
    // Simulação das pontuações
    cout << "USUÁRIO 1 (ADMIN):" << endl;
    cout << "  - Ontologia Score: 0.85 (ADMIN + CONTROL)" << endl;
    cout << "  - Trust Score: 0.90 (REQUESTED + INTERNAL + COMMON + TOGETHER)" << endl;
    cout << "  - Activity Score: 0.80 (padrão normal)" << endl;
    cout << "  - Context Score: 0.85 (contexto favorável)" << endl;
    cout << "  - Final Score: 0.85*0.3 + 0.90*0.3 + 0.80*0.2 + 0.85*0.2 = 0.855" << endl;
    cout << endl;
    
    cout << "USUÁRIO 2 (CHILD):" << endl;
    cout << "  - Ontologia Score: 0.45 (CHILD + CONTROL)" << endl;
    cout << "  - Trust Score: 0.30 (PERSONAL + EXTERNAL + UNCOMMON + ALONE)" << endl;
    cout << "  - Activity Score: 0.20 (padrão anômalo)" << endl;
    cout << "  - Context Score: 0.25 (contexto desfavorável)" << endl;
    cout << "  - Final Score: 0.45*0.3 + 0.30*0.3 + 0.20*0.2 + 0.25*0.2 = 0.315" << endl;
    cout << endl;
    
    cout << "RESULTADO:" << endl;
    cout << "  - Vencedor: USUÁRIO 1 (ADMIN)" << endl;
    cout << "  - Pontuação: 0.855 vs 0.315" << endl;
    cout << "  - Justificativa: Maior pontuação em todas as métricas" << endl;
    cout << endl;
}

void demonstrateEdgeCase() {
    /*
     * Caso limite: Usuários com pontuações similares
     * 
     * Usuário 1: ADULT, contexto médio, atividade normal
     * Usuário 2: ADULT, contexto médio, atividade normal
     * 
     * Neste caso, pequenas diferenças nas métricas determinam o vencedor
     */
    
    cout << "=== CASO LIMITE: PONTUAÇÕES SIMILARES ===" << endl;
    cout << endl;
    
    cout << "CENÁRIO: Dois adultos com perfis similares" << endl;
    cout << endl;
    
    cout << "USUÁRIO 1 (ADULT):" << endl;
    cout << "  - Ontologia Score: 0.70" << endl;
    cout << "  - Trust Score: 0.75" << endl;
    cout << "  - Activity Score: 0.80" << endl;
    cout << "  - Context Score: 0.70" << endl;
    cout << "  - Final Score: 0.725" << endl;
    cout << endl;
    
    cout << "USUÁRIO 2 (ADULT):" << endl;
    cout << "  - Ontologia Score: 0.70" << endl;
    cout << "  - Trust Score: 0.70" << endl;
    cout << "  - Activity Score: 0.75" << endl;
    cout << "  - Context Score: 0.75" << endl;
    cout << "  - Final Score: 0.720" << endl;
    cout << endl;
    
    cout << "RESULTADO:" << endl;
    cout << "  - Vencedor: USUÁRIO 1" << endl;
    cout << "  - Diferença: 0.005 (muito pequena)" << endl;
    cout << "  - Justificativa: Ligeira vantagem em Trust e Activity" << endl;
    cout << endl;
}

void demonstrateWeightAdjustment() {
    /*
     * Demonstração de como ajustar os pesos das métricas
     * para diferentes cenários de segurança
     */
    
    cout << "=== AJUSTE DE PESOS DAS MÉTRICAS ===" << endl;
    cout << endl;
    
    cout << "CONFIGURAÇÃO PADRÃO (Segurança Balanceada):" << endl;
    cout << "  - Ontologia: 30%" << endl;
    cout << "  - Confiança: 30%" << endl;
    cout << "  - Atividade: 20%" << endl;
    cout << "  - Contexto: 20%" << endl;
    cout << endl;
    
    cout << "CONFIGURAÇÃO ALTA SEGURANÇA:" << endl;
    cout << "  - Ontologia: 40% (mais restritiva)" << endl;
    cout << "  - Confiança: 35%" << endl;
    cout << "  - Atividade: 15%" << endl;
    cout << "  - Contexto: 10%" << endl;
    cout << endl;
    
    cout << "CONFIGURAÇÃO ALTA USABILIDADE:" << endl;
    cout << "  - Ontologia: 20%" << endl;
    cout << "  - Confiança: 25%" << endl;
    cout << "  - Atividade: 30% (mais adaptativa)" << endl;
    cout << "  - Contexto: 25%" << endl;
    cout << endl;
    
    cout << "CONFIGURAÇÃO CONTEXTO-CRÍTICA:" << endl;
    cout << "  - Ontologia: 25%" << endl;
    cout << "  - Confiança: 20%" << endl;
    cout << "  - Atividade: 20%" << endl;
    cout << "  - Contexto: 35% (mais sensível ao contexto)" << endl;
    cout << endl;
}

void demonstrateLearningEffect() {
    /*
     * Demonstração de como o sistema aprende com o tempo
     * e melhora a resolução de conflitos
     */
    
    cout << "=== EFEITO DO APRENDIZADO ===" << endl;
    cout << endl;
    
    cout << "FASE INICIAL (Sistema aprendendo):" << endl;
    cout << "  - Activity Score baseado em userLevel" << endl;
    cout << "  - Menos precisão na detecção de padrões" << endl;
    cout << "  - Maior dependência de ontologia e confiança" << endl;
    cout << endl;
    
    cout << "FASE MADURA (Sistema treinado):" << endl;
    cout << "  - Activity Score baseado em cadeias de Markov" << endl;
    cout << "  - Alta precisão na detecção de padrões" << endl;
    cout << "  - Resolução mais inteligente e adaptativa" << endl;
    cout << endl;
    
    cout << "EXEMPLO DE EVOLUÇÃO:" << endl;
    cout << "  - Usuário novo: Activity Score = 0.50 (userLevel)" << endl;
    cout << "  - Após 1 semana: Activity Score = 0.75 (padrões aprendidos)" << endl;
    cout << "  - Após 1 mês: Activity Score = 0.90 (padrões refinados)" << endl;
    cout << endl;
}

void runAllExamples() {
    demonstrateMultiMetricResolution();
    demonstrateEdgeCase();
    demonstrateWeightAdjustment();
    demonstrateLearningEffect();
    
    cout << "=== CONCLUSÃO ===" << endl;
    cout << endl;
    cout << "A abordagem multi-métrica inspirada em KRATOS oferece:" << endl;
    cout << "  ✓ Resolução inteligente de conflitos" << endl;
    cout << "  ✓ Consideração de múltiplos fatores" << endl;
    cout << "  ✓ Adaptabilidade ao comportamento dos usuários" << endl;
    cout << "  ✓ Configurabilidade para diferentes cenários" << endl;
    cout << "  ✓ Aprendizado contínuo e melhoria" << endl;
    cout << endl;
}

} // namespace ns3

// Função principal para demonstração
int main() {
    ns3::runAllExamples();
    return 0;
}

