# Monitoria de Grafos — Visualização e Algoritmos em 2D/3D



---

## Índice

- [Monitoria de Grafos — Visualização e Algoritmos em 2D/3D](#monitoria-de-grafos--visualização-e-algoritmos-em-2d3d)
  - [Índice](#índice)
  - [Instalação e Uso (Python)](#instalação-e-uso-python)
  - [Versão em C: Algoritmos Avançados de Grafos](#versão-em-c-algoritmos-avançados-de-grafos)
    - [**Principais Características**](#principais-características)
    - [**Algoritmos Implementados**](#algoritmos-implementados)
    - [**Instalação e Compilação**](#instalação-e-compilação)
    - [**Exemplos de Uso (C)**](#exemplos-de-uso-c)
      - [Ordenação Topológica](#ordenação-topológica)
      - [Detecção de Pontes e Articulações](#detecção-de-pontes-e-articulações)
      - [Emparelhamento Máximo em Bipartidos (Hopcroft-Karp)](#emparelhamento-máximo-em-bipartidos-hopcroft-karp)
  - [Funcionalidades](#funcionalidades)
  - [Tecnologias Utilizadas](#tecnologias-utilizadas)
  - [Contribuição](#contribuição)
  - [Licença](#licença)
  - [Notas Finais](#notas-finais)
  - [Referências](#referências)

---

## Instalação e Uso (Python)

1. Faça download do repositório:
   ```
   git clone https://github.com/thelostgus/grafo-monitoria
   ```
2. Acesse o repositório:
   ```
   cd grafo-monitoria
   ```
3. Crie um ambiente local Python:
   ```
   python -m venv venv
   ```
4. Ative o ambiente local:
   - No Windows:
     ```
     .\venv\Scripts\activate
     ```
   - No Linux:
     ```
     source venv/bin/activate
     ```
5. Instale as dependências:
   ```
   pip install -r requirements.txt
   ```
6. Execute o projeto:
   ```
   python main.py
   ```

> **Nota:** Após a primeira instalação, basta ativar o ambiente e executar o main. Futuras atualizações e melhorias nos algoritmos estão planejadas.

---

## Versão em C: Algoritmos Avançados de Grafos

A implementação em C foi desenvolvida para máxima performance, flexibilidade e segurança, suportando grandes grafos e algoritmos clássicos e modernos para análise estrutural, combinatória e de robustez de redes.

### **Principais Características**

- **Suporte a múltiplos tipos de pesos:**  
  Generalização do tipo de peso das arestas (`int`, `float`, `double`) via `typedef` e macros. Basta alterar uma linha para mudar o tipo de peso em todo o projeto, com valor simbólico de infinito adaptado automaticamente.
- **Estruturas de dados eficientes:**  
  Grafos representados por listas de adjacência dinâmicas, com vértices indexados de 0 a `num_vertices-1`.
- **Documentação Doxygen e padrões CERT C:**  
  Todo o código é documentado em estilo Doxygen, com exemplos de uso, e segue rigorosamente o SEI CERT C Coding Standard para segurança, portabilidade e robustez.

### **Algoritmos Implementados**

- **Ordenação Topológica:**  
  Algoritmo de Kahn para DAGs, útil em análise de precedência, dependências e planejamento de tarefas.
- **Detecção de Pontes e Articulações:**  
  Algoritmo de Tarjan para identificar arestas e vértices críticos (robustez de redes).
- **Componentes Biconexas:**  
  Identificação de subconjuntos maximamente 2-conexos, essenciais para análise de conectividade.
- **Teste de Bipartição:**  
  Verificação e coloração 2-partida, fundamental para matching, coloração e análise de grafos bipartidos.
- **Emparelhamento Máximo em Bipartidos (Hopcroft-Karp):**  
  Algoritmo eficiente (O(E√V)) para encontrar o maior conjunto de pares disjuntos em grafos bipartidos.
- **Algoritmos clássicos:**  
  BFS, DFS, Dijkstra, Bellman-Ford, Floyd-Warshall, Kruskal, Hierholzer (caminho euleriano), Edmonds-Karp (fluxo máximo), entre outros.

---

### **Instalação e Compilação**

**Pré-requisitos:**
- Compilador C padrão (GCC, Clang ou equivalente)
- Sistema operacional compatível (Linux, Windows, macOS)

**Compilação:**
```sh
gcc -std=c17 -Wall -Wextra -O2 -o grafo grafo.c
```

**Execução:**
```sh
./grafo
```
> Consulte o código-fonte para exemplos de uso de cada algoritmo.

---

### **Exemplos de Uso (C)**

#### Ordenação Topológica
```c
size_t *ordem = topological_sort(g);
if (ordem) {
    for (size_t i = 0; i < g->num_vertices; i++)
        printf("%zu ", ordem[i]);
    free(ordem);
}
```

#### Detecção de Pontes e Articulações
```c
Edge *bridges = NULL; size_t nbridges = 0;
size_t *artics = NULL; size_t nartics = 0;
if (detect_bridges_articulations(g, &bridges, &nbridges, &artics, &nartics) == 0) {
    // Imprime pontes e articulações
    free(bridges); free(artics);
}
```

#### Emparelhamento Máximo em Bipartidos (Hopcroft-Karp)
```c
size_t *matching = NULL;
int max_matching = hopcroft_karp(g, part_u_size, &matching);
if (max_matching >= 0) {
    // Imprime pares emparelhados
    free(matching);
}
```

> Consulte o código-fonte para exemplos completos e documentação detalhada de cada função.

---

## Funcionalidades

| Funcionalidade                        | Python | C      |
|---------------------------------------|:------:|:------:|
| Visualização 2D/3D                    |   ✔    |   —    |
| BFS/DFS                               |   ✔    |   ✔    |
| Caminhos mínimos (Dijkstra, etc.)     |   ✔    |   ✔    |
| Ordenação Topológica                  |   —    |   ✔    |
| Detecção de Pontes/Articulações       |   —    |   ✔    |
| Componentes Biconexas                 |   —    |   ✔    |
| Teste de Bipartição                   |   —    |   ✔    |
| Emparelhamento Máximo (Hopcroft-Karp) |   —    |   ✔    |
| Fluxo Máximo                          |   —    |   ✔    |
| Árvore Geradora Mínima (Kruskal)      |   ✔    |   ✔    |

---

## Tecnologias Utilizadas

- **Python 3.x**: Visualização, prototipagem e algoritmos básicos.
- **C (C11/C17)**: Algoritmos avançados, performance e manipulação eficiente de grandes grafos.
- **Bibliotecas padrão**: stdio.h, stdlib.h, stdbool.h, string.h, limits.h, etc.

---

## Contribuição

Contribuições são bem-vindas! Para contribuir, siga as etapas:

1. Fork este repositório.
2. Crie uma branch para sua feature ou correção.
3. Envie um pull request detalhando as alterações propostas.

Consulte o arquivo CONTRIBUTING.md para mais informações.

---

## Licença

Este projeto está licenciado sob a [MIT License](LICENSE.md).

---

## Notas Finais

- O README.md é atualizado continuamente para refletir as melhorias e novas funcionalidades do projeto.
- Para dúvidas, sugestões ou relatos de bugs, abra uma issue ou entre em contato com os mantenedores.

---

## Referências

- SEI CERT C Coding Standard  
- Effective C — An Introduction to Professional C Programming  
- Guia de boas práticas para README.md  
- Documentação e exemplos de algoritmos clássicos de grafos

---

> **Key Takeaway:**  
**A versão em C do projeto Monitoria de Grafos oferece máxima performance, flexibilidade e segurança, com suporte a múltiplos tipos de pesos e algoritmos avançados, documentados e em conformidade com os mais altos padrões de qualidade e segurança em C.**

---

> _Para detalhes completos, consulte a documentação inline do código-fonte egrafo.c