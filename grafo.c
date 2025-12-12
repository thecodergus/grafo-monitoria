#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <limits.h>

#define GRAFO_MAX_VERTICES 10000

/*
 * Estrutura de dados para representar um grafo em C.
 * Utiliza listas de adjacência com vetores dinâmicos.
 * Assume vértices enumerados de 0 a num_vertices-1.
 */

typedef struct
{
    size_t **adj;        // Listas de adjacência (cada posição um array dinâmico de vizinhos)
    size_t *adj_size;    // Vetor com o tamanho da lista de adjacência de cada vértice
    size_t num_vertices; // Número de vértices
    bool directed;       // Indica se o grafo é direcionado
} Graph;

/**
 * @brief Estrutura que representa uma aresta entre dois vértices.
 * @note v1 e v2 devem ser índices válidos de vértices (0 <= v < num_vertices).
 * @note Para máxima portabilidade, prefira size_t em novos projetos.
 */
typedef struct
{
    int v1;
    int v2;
} Edge;

/******************** FUNÇÕES AUXILIARES DE MEMÓRIA ********************/

/**
 * @brief Checa se a multiplicação a * b causa overflow.
 * @param a Primeiro operando
 * @param b Segundo operando
 * @return true se houver overflow, false caso contrário
 * @note Regras CERT: INT30-C, MEM35-C, MEM07-C
 */
static bool _checa_overflow(size_t a, size_t b)
{
    return (a > 0 && b > SIZE_MAX / a);
}

/**
 * @brief Verifica se a alocação foi bem-sucedida.
 *        Em caso de falha, imprime mensagem de erro detalhada e retorna NULL.
 * @param ptr Ponteiro retornado por função de alocação
 * @param func Nome da função de alocação (para depuração)
 * @param size Tamanho solicitado (ou 0 se não aplicável)
 * @return O ponteiro original se não for NULL, ou NULL em caso de falha
 * @note Não aborta o programa, permitindo tratamento flexível de erro.
 * @note Regras CERT: ERR33-C, MEM35-C
 */
static void *check_allocation(void *ptr, const char *func, size_t size)
{
    if (ptr == NULL)
    {
        fprintf(stderr, "[ERRO] %s: falha ao alocar %zu bytes\n", func, size);
        // Não aborta: permite ao chamador tratar o erro
    }
    return ptr;
}

/**
 * @brief Realoca memória de forma segura, prevenindo vazamento e double free.
 *        Não libera o ponteiro original em caso de falha, conforme CERT C.
 * @param ptr Ponteiro para o bloco de memória a ser realocado
 * @param new_size Novo tamanho desejado
 * @return Ponteiro para a nova área de memória, ou NULL em caso de erro
 * @note O ponteiro original permanece válido se realloc falhar.
 * @note Regras CERT: MEM31-C, MEM35-C, ERR33-C
 */
static void *safe_realloc(void *ptr, size_t new_size)
{
    if (new_size == 0)
    {
        // Comportamento portável: libera ptr e retorna NULL
        free(ptr);
        return NULL;
    }
    void *new_ptr = realloc(ptr, new_size);
    if (new_ptr == NULL)
    {
        fprintf(stderr, "[ERRO] safe_realloc: falha ao realocar para %zu bytes\n", new_size);
        // Não libera ptr: o chamador deve decidir como proceder
        return NULL;
    }
    return new_ptr;
}

/**
 * @brief Aloca memória inicializada com zeros, checando overflow e erros.
 * @param count Número de elementos
 * @param size Tamanho de cada elemento
 * @return Ponteiro para a memória alocada, ou NULL em caso de erro
 * @note Regras CERT: MEM07-C, MEM35-C, MEM04-C
 */
static void *safe_calloc(size_t count, size_t size)
{
    if (count == 0 || size == 0)
    {
        // Evita comportamento indefinido de calloc(0, x) ou calloc(x, 0)
        return NULL;
    }
    if (_checa_overflow(count, size))
    {
        fprintf(stderr, "[ERRO] safe_calloc: overflow no cálculo de %zu * %zu bytes\n", count, size);
        return NULL;
    }
    void *ptr = calloc(count, size);
    return check_allocation(ptr, "safe_calloc", count * size);
}

/**
 * @brief Aloca memória de tamanho especificado, checando erros e tamanho zero.
 * @param size Tamanho em bytes
 * @return Ponteiro para a memória alocada, ou NULL em caso de erro
 * @note Regras CERT: MEM04-C, MEM35-C
 */
static void *safe_malloc(size_t size)
{
    if (size == 0)
    {
        // Evita comportamento indefinido de malloc(0)
        return NULL;
    }
    void *ptr = malloc(size);
    return check_allocation(ptr, "safe_malloc", size);
}

/******************** FUNÇÕES DO GRAFO ********************/

/**
 * @brief Cria um grafo seguro e vazio.
 * @param num_vertices Número de vértices (1..GRAFO_MAX_VERTICES)
 * @param directed Se o grafo é dirigido
 * @return Ponteiro para o grafo criado ou NULL em caso de erro
 * @pre num_vertices > 0 && num_vertices <= GRAFO_MAX_VERTICES
 * @post Grafo inicializado com listas de adjacência vazias
 * @note CERT C: API00-C, MEM35-C, ERR33-C
 */
Graph *create_graph(size_t num_vertices, bool directed)
{
    if (num_vertices == 0 || num_vertices > GRAFO_MAX_VERTICES)
    {
        fprintf(stderr, "[ERRO] create_graph: Número de vértices inválido (%zu). Deve ser entre 1 e %d.\n", num_vertices, GRAFO_MAX_VERTICES);
        return NULL;
    }
    if (_checa_overflow(num_vertices, sizeof(size_t *)))
    {
        fprintf(stderr, "[ERRO] create_graph: Overflow na alocação de ponteiros de adjacência.\n");
        return NULL;
    }

    Graph *g = (Graph *)safe_malloc(sizeof(Graph));
    if (!g)
        return NULL;

    g->num_vertices = num_vertices;
    g->directed = directed;

    g->adj = (size_t **)safe_calloc(num_vertices, sizeof(size_t *));
    if (!g->adj)
    {
        free(g);
        return NULL;
    }

    g->adj_size = (size_t *)safe_calloc(num_vertices, sizeof(size_t));
    if (!g->adj_size)
    {
        free(g->adj);
        free(g);
        return NULL;
    }

    // safe_calloc já inicializa adjacências para NULL e adj_size para 0
    return g;
}

/**
 * @brief Adiciona uma aresta entre os vértices src e dest.
 *        Para grafos não-direcionados, adiciona a aresta (dest, src) também.
 * @param g Ponteiro para o grafo
 * @param src Vértice de origem
 * @param dest Vértice de destino
 * @return 0 em caso de sucesso, -1 se o grafo for NULL, -2 se os vértices forem inválidos,
 *         -3 se houver overflow na alocação, -4 se a realocação falhar.
 * @pre g != NULL, src < g->num_vertices, dest < g->num_vertices
 * @note CERT C: API00-C, ARR30-C, MEM35-C, ERR33-C
 */
int add_edge(Graph *g, size_t src, size_t dest)
{
    if (!g)
    {
        fprintf(stderr, "[ERRO] add_edge: Grafo é NULL.\n");
        return -1;
    }
    if (src >= g->num_vertices || dest >= g->num_vertices)
    {
        fprintf(stderr, "[ERRO] add_edge: Índice de vértice inválido (src=%zu, dest=%zu, num_vertices=%zu).\n", src, dest, g->num_vertices);
        return -2;
    }

    // (Opcional) Evita duplicatas:
    // for (size_t i = 0; i < g->adj_size[src]; ++i)
    //     if (g->adj[src][i] == dest) return 0;

    if (_checa_overflow(g->adj_size[src] + 1, sizeof(size_t)))
    {
        fprintf(stderr, "[ERRO] add_edge: Overflow no cálculo do novo tamanho para src %zu.\n", src);
        return -3;
    }
    size_t *new_adj_src = (size_t *)safe_realloc(g->adj[src], (g->adj_size[src] + 1) * sizeof(size_t));
    if (!new_adj_src)
    {
        fprintf(stderr, "[ERRO] add_edge: Falha ao realocar lista de adjacência para src %zu.\n", src);
        return -4;
    }
    g->adj[src] = new_adj_src;
    g->adj[src][g->adj_size[src]] = dest;
    g->adj_size[src]++;

    if (!g->directed)
    {
        if (_checa_overflow(g->adj_size[dest] + 1, sizeof(size_t)))
        {
            fprintf(stderr, "[ERRO] add_edge: Overflow no cálculo do novo tamanho para dest %zu.\n", dest);
            // (Opcional) rollback: desfaz a adição anterior
            g->adj_size[src]--;
            return -3;
        }
        size_t *new_adj_dest = (size_t *)safe_realloc(g->adj[dest], (g->adj_size[dest] + 1) * sizeof(size_t));
        if (!new_adj_dest)
        {
            fprintf(stderr, "[ERRO] add_edge: Falha ao realocar lista de adjacência para dest %zu.\n", dest);
            g->adj_size[src]--; // rollback
            return -4;
        }
        g->adj[dest] = new_adj_dest;
        g->adj[dest][g->adj_size[dest]] = src;
        g->adj_size[dest]++;
    }
    return 0;
}

/**
 * @brief Remove uma aresta entre os vértices src e dest.
 *        Para grafos não-direcionados, remove a aresta (dest, src) também.
 * @param g Ponteiro para o grafo
 * @param src Vértice de origem
 * @param dest Vértice de destino
 * @return 0 em caso de sucesso, -1 se o grafo for NULL, -2 se os vértices forem inválidos,
 *         -3 se a aresta não for encontrada, -4 se a realocação falhar.
 * @pre g != NULL, src < g->num_vertices, dest < g->num_vertices
 * @note CERT C: API00-C, ARR30-C, MEM35-C, ERR33-C
 */
int remove_edge(Graph *g, size_t src, size_t dest)
{
    if (!g)
    {
        fprintf(stderr, "[ERRO] remove_edge: Grafo é NULL.\n");
        return -1;
    }
    if (src >= g->num_vertices || dest >= g->num_vertices)
    {
        fprintf(stderr, "[ERRO] remove_edge: Índice de vértice inválido (src=%zu, dest=%zu, num_vertices=%zu).\n", src, dest, g->num_vertices);
        return -2;
    }

    bool found_src_dest = false;
    for (size_t i = 0; i < g->adj_size[src]; i++)
    {
        if (g->adj[src][i] == dest)
        {
            for (size_t j = i; j + 1 < g->adj_size[src]; j++)
                g->adj[src][j] = g->adj[src][j + 1];
            g->adj_size[src]--;
            size_t *new_adj_src = (size_t *)safe_realloc(g->adj[src], g->adj_size[src] * sizeof(size_t));
            if (!new_adj_src && g->adj_size[src] > 0)
            {
                fprintf(stderr, "[ERRO] remove_edge: Falha ao realocar lista de adjacência para src %zu.\n", src);
                return -4;
            }
            g->adj[src] = new_adj_src;
            found_src_dest = true;
            break;
        }
    }

    if (!found_src_dest)
    {
        return -3; // Aresta não encontrada
    }

    if (!g->directed)
    {
        bool found_dest_src = false;
        for (size_t i = 0; i < g->adj_size[dest]; i++)
        {
            if (g->adj[dest][i] == src)
            {
                for (size_t j = i; j + 1 < g->adj_size[dest]; j++)
                    g->adj[dest][j] = g->adj[dest][j + 1];
                g->adj_size[dest]--;
                size_t *new_adj_dest = (size_t *)safe_realloc(g->adj[dest], g->adj_size[dest] * sizeof(size_t));
                if (!new_adj_dest && g->adj_size[dest] > 0)
                {
                    fprintf(stderr, "[ERRO] remove_edge: Falha ao realocar lista de adjacência para dest %zu.\n", dest);
                    return -4;
                }
                g->adj[dest] = new_adj_dest;
                found_dest_src = true;
                break;
            }
        }
        if (!found_dest_src)
        {
            fprintf(stderr, "[AVISO] remove_edge: Aresta (%zu, %zu) removida, mas (%zu, %zu) não encontrada em grafo não-direcionado.\n", src, dest, dest, src);
        }
    }
    return 0;
}

/**
 * @brief Retorna o número de vértices do grafo.
 * @param g Ponteiro constante para o grafo.
 * @return O número de vértices, ou 0 se g for NULL.
 * @pre g != NULL
 * @note Em caso de erro, imprime mensagem em stderr.
 * @note CERT C: DCL13-C, API00-C
 */
size_t get_num_vertices(const Graph *g)
{
    if (g == NULL)
    {
        fprintf(stderr, "[ERRO] get_num_vertices: Grafo é NULL.\n");
        return 0;
    }
    return g->num_vertices;
}

/**
 * @brief Retorna o grau (número de arestas incidentes/saindo) de um vértice.
 * @param g Ponteiro constante para o grafo.
 * @param vertex Índice do vértice (0 <= vertex < g->num_vertices).
 * @return O grau do vértice, ou SIZE_MAX em caso de erro (grafo NULL ou índice inválido).
 * @pre g != NULL, vertex < g->num_vertices
 * @note Em caso de erro, imprime mensagem em stderr.
 * @note CERT C: DCL13-C, API00-C, ARR30-C
 */
size_t get_degree(const Graph *g, size_t vertex)
{
    if (g == NULL)
    {
        fprintf(stderr, "[ERRO] get_degree: Grafo é NULL.\n");
        return SIZE_MAX;
    }
    if (vertex >= g->num_vertices)
    {
        fprintf(stderr, "[ERRO] get_degree: Índice de vértice inválido (%zu). Deve ser menor que %zu.\n", vertex, g->num_vertices);
        return SIZE_MAX;
    }
    return g->adj_size[vertex];
}

/**
 * @brief Executa busca em largura (BFS) a partir de um vértice inicial.
 * @param[in] g Ponteiro constante para o grafo.
 * @param[in] start Vértice inicial (0 <= start < g->num_vertices).
 * @param[out] visited_count (Opcional) Ponteiro para size_t que recebe o número de vértices visitados.
 * @return Ponteiro para array dinâmico com a ordem de visita, ou NULL em caso de erro.
 * @pre g != NULL, start < g->num_vertices
 * @post O usuário é responsável por liberar a memória retornada.
 * @note Em caso de erro, imprime mensagem em stderr e retorna NULL.
 * @note CERT C: API00-C, ARR30-C, MEM35-C, MEM31-C, ERR33-C, DCL13-C
 */
int *bfs(const Graph *g, size_t start /*, size_t *visited_count */)
{
    if (g == NULL)
    {
        fprintf(stderr, "[ERRO] bfs: Grafo é NULL.\n");
        return NULL;
    }
    if (start >= g->num_vertices)
    {
        fprintf(stderr, "[ERRO] bfs: Índice de vértice inicial inválido (%zu).\n", start);
        return NULL;
    }
    if (g->num_vertices == 0 || g->num_vertices > SIZE_MAX / sizeof(int))
    {
        fprintf(stderr, "[ERRO] bfs: Número de vértices inválido ou overflow.\n");
        return NULL;
    }

    bool *visited = (bool *)safe_calloc(g->num_vertices, sizeof(bool));
    if (!visited)
        return NULL;
    int *result = (int *)safe_malloc(g->num_vertices * sizeof(int));
    if (!result)
    {
        free(visited);
        return NULL;
    }
    int *queue = (int *)safe_malloc(g->num_vertices * sizeof(int));
    if (!queue)
    {
        free(result);
        free(visited);
        return NULL;
    }

    size_t front = 0, rear = 0, count = 0;
    visited[start] = true;
    queue[rear++] = (int)start;

    while (front < rear)
    {
        int v = queue[front++];
        result[count++] = v;

        for (size_t i = 0; i < g->adj_size[v]; i++)
        {
            int neigh = (int)g->adj[v][i];
            if ((size_t)neigh >= g->num_vertices)
            {
                fprintf(stderr, "[AVISO] bfs: Vizinho inválido (%d) encontrado para vértice %d. Ignorando.\n", neigh, v);
                continue;
            }
            if (!visited[neigh])
            {
                visited[neigh] = true;
                queue[rear++] = neigh;
            }
        }
    }

    free(queue);
    free(visited);

    // (Opcional) Se desejar retornar o número de visitados:
    // if (visited_count) *visited_count = count;

    // (Opcional) Redimensionar result para count:
    // int *resized = safe_realloc(result, count * sizeof(int));
    // if (resized) result = resized;

    return result;
}

/**
 * @brief Executa busca em profundidade (DFS) iterativa a partir de um vértice inicial.
 * @param[in] g Ponteiro constante para o grafo.
 * @param[in] start Vértice inicial (0 <= start < g->num_vertices).
 * @param[out] visited_count (Opcional) Ponteiro para size_t que recebe o número de vértices visitados.
 * @return Ponteiro para array dinâmico com a ordem de visita, ou NULL em caso de erro.
 * @pre g != NULL, start < g->num_vertices
 * @post O usuário é responsável por liberar a memória retornada.
 * @note Em caso de erro, imprime mensagem em stderr e retorna NULL.
 * @note CERT C: API00-C, ARR30-C, MEM35-C, MEM31-C, ERR33-C, DCL13-C
 */
int *dfs(const Graph *g, size_t start /*, size_t *visited_count */)
{
    if (g == NULL)
    {
        fprintf(stderr, "[ERRO] dfs: Grafo é NULL.\n");
        return NULL;
    }
    if (start >= g->num_vertices)
    {
        fprintf(stderr, "[ERRO] dfs: Índice de vértice inicial inválido (%zu).\n", start);
        return NULL;
    }
    if (g->num_vertices == 0 || g->num_vertices > SIZE_MAX / sizeof(int))
    {
        fprintf(stderr, "[ERRO] dfs: Número de vértices inválido ou overflow.\n");
        return NULL;
    }

    bool *visited = (bool *)safe_calloc(g->num_vertices, sizeof(bool));
    if (!visited)
        return NULL;
    int *stack = (int *)safe_malloc(g->num_vertices * sizeof(int));
    if (!stack)
    {
        free(visited);
        return NULL;
    }
    int *path = (int *)safe_malloc(g->num_vertices * sizeof(int));
    if (!path)
    {
        free(stack);
        free(visited);
        return NULL;
    }

    size_t top = -1;
    stack[++top] = (int)start;
    size_t count = 0;

    while (top >= 0)
    {
        int v = stack[top--];
        if ((size_t)v >= g->num_vertices)
        {
            fprintf(stderr, "[AVISO] dfs: Vértice inválido (%d) desempilhado. Ignorando.\n", v);
            continue;
        }
        if (!visited[v])
        {
            visited[v] = true;
            path[count++] = v;
            // Adiciona vizinhos na pilha (pode inverter a ordem se desejado)
            for (size_t i = 0; i < g->adj_size[v]; i++)
            {
                int neigh = (int)g->adj[v][i];
                if ((size_t)neigh >= g->num_vertices)
                {
                    fprintf(stderr, "[AVISO] dfs: Vizinho inválido (%d) encontrado para vértice %d. Ignorando.\n", neigh, v);
                    continue;
                }
                if (!visited[neigh])
                {
                    stack[++top] = neigh;
                }
            }
        }
    }

    free(stack);
    free(visited);

    // (Opcional) Se desejar retornar o número de visitados:
    // if (visited_count) *visited_count = count;

    // (Opcional) Redimensionar path para count:
    // int *resized = safe_realloc(path, count * sizeof(int));
    // if (resized) path = resized;

    return path;
}

/**
 * @brief Função auxiliar recursiva para detecção de ciclo em grafos não direcionados.
 *        Utiliza DFS para verificar se um vizinho já visitado não é o pai do vértice atual.
 *
 * @param[in]  g      Ponteiro constante para o grafo. Não deve ser NULL.
 * @param[in]  u      Vértice atual sendo visitado (0 <= u < g->num_vertices).
 * @param[in,out] visited Array de booleanos para marcar vértices visitados.
 *                        Deve ter g->num_vertices elementos válidos.
 * @param[in]  parent Vértice que chamou 'u' na recursão (para evitar considerar
 *                    aresta de volta como ciclo). Use (ssize_t)-1 para o vértice inicial.
 *
 * @return true se um ciclo for detectado, false caso contrário.
 *
 * @pre g != NULL && visited != NULL && u < g->num_vertices
 * @pre g->adj != NULL && g->adj_size != NULL
 * @pre Para todo i: g->adj[u][i] < g->num_vertices (vizinhos válidos)
 *
 * @post visited[u] = true
 * @post Se retornar true, existe um ciclo no componente conectado de u
 *
 * @note Esta função é interna e deve ser chamada por uma função pública de detecção de ciclo.
 * @note Complexidade: O(V + E) onde V = vértices, E = arestas
 * @note Espaço na pilha: O(V) no pior caso (grafo linear)
 * @warning Em grafos muito profundos pode causar stack overflow. Considere versão iterativa para aplicações críticas.
 * @see CERT C: API00-C, ARR30-C, DCL13-C, REC01-C, ERR33-C
 */
static bool has_cycle_util(const Graph *g, size_t u, bool visited[], size_t parent)
{
    // Validação defensiva de parâmetros (API00-C)
    if (g == NULL || visited == NULL || u >= g->num_vertices)
    {
        fprintf(stderr, "[ERRO] has_cycle_util: Parâmetros inválidos (g=%p, u=%zu, visited=%p, num_vertices=%zu).\n",
                (void *)g, u, (void *)visited, g ? g->num_vertices : 0);
        return false;
    }

    // Validação adicional da estrutura do grafo
    if (g->adj == NULL || g->adj_size == NULL)
    {
        fprintf(stderr, "[ERRO] has_cycle_util: Estrutura de grafo inválida (adj=%p, adj_size=%p).\n",
                (void *)g->adj, (void *)g->adj_size);
        return false;
    }

    // Marca o vértice atual como visitado
    visited[u] = true;

    // Explora todos os vizinhos do vértice atual
    for (size_t i = 0; i < g->adj_size[u]; i++)
    {
        // Validação de lista de adjacência (ARR30-C)
        if (g->adj[u] == NULL)
        {
            fprintf(stderr, "[AVISO] has_cycle_util: Lista de adjacência nula para vértice %zu. Ignorando.\n", u);
            continue;
        }

        size_t v = g->adj[u][i];

        // Validação do vizinho (ARR30-C)
        if (v >= g->num_vertices)
        {
            fprintf(stderr, "[AVISO] has_cycle_util: Vizinho inválido (%zu) encontrado para vértice %zu. Ignorando.\n", v, u);
            continue;
        }

        if (!visited[v])
        {
            if (has_cycle_util(g, v, visited, (size_t)u))
                return true;
        }
        else if ((size_t)v != parent)
        {
            // Se o vizinho já foi visitado e não é o pai, há ciclo
            return true;
        }
    }

    // Nenhum ciclo encontrado a partir deste vértice
    return false;
}
/**
 * @brief Verifica se o grafo possui ciclo (apenas para grafos não-direcionados).
 *
 * @param[in] g Ponteiro constante para o grafo.
 * @return true se o grafo possui ciclo, false caso contrário ou em caso de erro.
 *
 * @pre g != NULL && g->num_vertices > 0
 * @note O algoritmo é válido apenas para grafos não-direcionados.
 * @note O usuário deve garantir que o grafo está corretamente inicializado.
 * @note Em caso de erro, imprime mensagem em stderr e retorna false.
 * @see has_cycle_util
 * @see SEI CERT C: API00-C, ARR30-C, MEM35-C, ERR33-C, DCL13-C
 */
bool has_cycle(const Graph *g)
{
    // Validação de ponteiro e estrutura (API00-C)
    if (g == NULL)
    {
        fprintf(stderr, "[ERRO] has_cycle: Grafo é NULL.\n");
        return false;
    }
    if (g->num_vertices == 0 || g->adj == NULL || g->adj_size == NULL)
    {
        fprintf(stderr, "[ERRO] has_cycle: Grafo malformado ou sem vértices.\n");
        return false;
    }
    if (g->num_vertices > SIZE_MAX / sizeof(bool))
    {
        fprintf(stderr, "[ERRO] has_cycle: Overflow no cálculo do vetor visited.\n");
        return false;
    }

    // Alocação segura do vetor de visitados (MEM35-C)
    bool *visited = (bool *)safe_calloc(g->num_vertices, sizeof(bool));
    if (!visited)
    {
        fprintf(stderr, "[ERRO] has_cycle: Falha ao alocar vetor visited.\n");
        return false;
    }

    // Percorre todos os vértices para cobrir componentes desconexos
    for (size_t v = 0; v < g->num_vertices; v++)
    {
        if (!visited[v])
        {
            // Usa SIZE_MAX como sentinela para "sem pai" (eliminando ssize_t e -1)
            if (has_cycle_util(g, v, visited, SIZE_MAX))
            {
                free(visited);
                return true;
            }
        }
    }

    free(visited);
    return false;
}

typedef struct
{
    int v1, v2;
} Edge;

/**
 * @brief Retorna um array dinâmico com todas as arestas do grafo.
 * @param[in]  g         Ponteiro constante para o grafo.
 * @param[in]  as_tuple  Parâmetro mantido por compatibilidade (não afeta a saída).
 * @param[out] num_edges Ponteiro para size_t que receberá o número de arestas.
 * @return Ponteiro para array dinâmico de Edge, ou NULL em caso de erro.
 * @pre g != NULL, num_edges != NULL
 * @post O usuário é responsável por liberar o array retornado.
 * @note Em caso de erro, imprime mensagem em stderr, retorna NULL e *num_edges = 0.
 * @note CERT C: API00-C, ARR30-C, MEM35-C, ERR33-C, DCL13-C
 */
Edge *get_edges(const Graph *g, bool as_tuple, size_t *num_edges)
{
    // Validação dos parâmetros de entrada (API00-C)
    if (g == NULL || num_edges == NULL)
    {
        fprintf(stderr, "[ERRO] get_edges: Parâmetro(s) nulo(s).\n");
        if (num_edges)
            *num_edges = 0;
        return NULL;
    }
    *num_edges = 0;

    if (g->num_vertices == 0 || g->adj == NULL || g->adj_size == NULL)
    {
        fprintf(stderr, "[ERRO] get_edges: Grafo malformado ou sem vértices.\n");
        return NULL;
    }
    if (g->num_vertices > SIZE_MAX / sizeof(Edge))
    {
        fprintf(stderr, "[ERRO] get_edges: Overflow no cálculo da capacidade de arestas.\n");
        return NULL;
    }

    // Estimativa superdimensionada: n^2 (pior caso)
    size_t capacity = g->num_vertices * g->num_vertices;
    if (capacity == 0 || capacity > SIZE_MAX / sizeof(Edge))
    {
        fprintf(stderr, "[ERRO] get_edges: Capacidade de arestas inválida ou overflow.\n");
        return NULL;
    }

    Edge *edges = (Edge *)safe_malloc(capacity * sizeof(Edge));
    if (!edges)
    {
        fprintf(stderr, "[ERRO] get_edges: Falha ao alocar array de arestas.\n");
        return NULL;
    }

    bool *processed = NULL;
    if (!g->directed)
    {
        if (capacity > SIZE_MAX / sizeof(bool))
        {
            fprintf(stderr, "[ERRO] get_edges: Overflow no cálculo do vetor processed.\n");
            free(edges);
            return NULL;
        }
        processed = (bool *)safe_calloc(capacity, sizeof(bool));
        if (!processed)
        {
            fprintf(stderr, "[ERRO] get_edges: Falha ao alocar vetor processed.\n");
            free(edges);
            return NULL;
        }
    }

    for (size_t v = 0; v < g->num_vertices; v++)
    {
        if (g->adj[v] == NULL)
            continue;
        for (size_t i = 0; i < g->adj_size[v]; i++)
        {
            int w = (int)g->adj[v][i];
            if ((size_t)w >= g->num_vertices)
            {
                fprintf(stderr, "[AVISO] get_edges: Vizinho inválido (%d) em v=%zu. Ignorando.\n", w, v);
                continue;
            }
            if (g->directed)
            {
                if (*num_edges >= capacity)
                {
                    fprintf(stderr, "[ERRO] get_edges: Capacidade de arestas excedida.\n");
                    if (processed)
                        free(processed);
                    free(edges);
                    *num_edges = 0;
                    return NULL;
                }
                edges[*num_edges].v1 = (int)v;
                edges[*num_edges].v2 = w;
                (*num_edges)++;
            }
            else
            {
                int min = ((int)v < w) ? (int)v : w;
                int max = ((int)v < w) ? w : (int)v;
                size_t index = (size_t)min * g->num_vertices + (size_t)max;
                if (index >= capacity)
                {
                    fprintf(stderr, "[AVISO] get_edges: Índice processed fora do limite (%zu).\n", index);
                    continue;
                }
                if (!processed[index])
                {
                    if (*num_edges >= capacity)
                    {
                        fprintf(stderr, "[ERRO] get_edges: Capacidade de arestas excedida.\n");
                        free(processed);
                        free(edges);
                        *num_edges = 0;
                        return NULL;
                    }
                    edges[*num_edges].v1 = min;
                    edges[*num_edges].v2 = max;
                    processed[index] = true;
                    (*num_edges)++;
                }
            }
        }
    }

    // Redimensiona edges para o tamanho real (MEM35-C)
    if (*num_edges > 0)
    {
        Edge *resized = (Edge *)safe_realloc(edges, (*num_edges) * sizeof(Edge));
        if (resized)
        {
            edges = resized;
        }
        else
        {
            fprintf(stderr, "[AVISO] get_edges: Falha ao redimensionar array de arestas. Mantendo tamanho superestimado.\n");
            // edges ainda é válido, mas ocupa mais memória que o necessário
        }
    }
    else
    {
        // Nenhuma aresta encontrada
        free(edges);
        edges = NULL;
    }

    if (processed)
        free(processed);
    return edges;
}

/**
 * @brief Imprime informações detalhadas sobre o grafo, incluindo vértices e arestas.
 *        Similar ao método __str__ do Python para representação de objetos.
 *
 * @param[in] g Ponteiro constante para o grafo.
 * @pre g != NULL && g->adj != NULL && g->adj_size != NULL
 * @post Imprime a representação do grafo no stdout. Não altera o estado do grafo.
 * @return Void
 * @note Se o grafo for vazio (g->num_vertices == 0), imprime mensagem informativa.
 * @note Se funções auxiliares falharem, imprime mensagem de erro em stderr.
 * @note Complexidade: O(V + E), onde V = número de vértices, E = número de arestas.
 * @warning Não é thread-safe se o grafo puder ser modificado por outras threads durante a execução.
 * @see get_edges
 * @code
 * print_graph(g);
 * @endcode
 * @note CERT C: API00-C, DCL13-C, ERR33-C, ARR30-C, MEM35-C
 */
void print_graph(const Graph *g)
{
    // Validação de ponteiro e estrutura (API00-C, DCL13-C)
    if (g == NULL)
    {
        fprintf(stderr, "[ERRO] print_graph: Grafo é NULL. Não é possível imprimir.\n");
        return;
    }
    if (g->adj == NULL || g->adj_size == NULL)
    {
        fprintf(stderr, "[ERRO] print_graph: Estrutura interna do grafo é inválida (adj ou adj_size é NULL).\n");
        return;
    }
    if (g->num_vertices == 0)
    {
        printf("Grafo vazio (0 vértices).\n");
        return;
    }

    printf("--- Informações do Grafo ---\n");

    // Imprime vértices
    printf("Vértices (%zu): ", g->num_vertices);
    for (size_t i = 0; i < g->num_vertices; i++)
    {
        printf("%zu ", i);
    }
    printf("\n");

    // Obtém e imprime arestas
    size_t num_edges = 0;
    Edge *edges = get_edges(g, true, &num_edges);

    printf("Arestas (%zu): ", num_edges);
    if (edges != NULL)
    {
        for (size_t i = 0; i < num_edges; i++)
        {
            printf("(%d, %d)%s", edges[i].v1, edges[i].v2, (i == num_edges - 1) ? "" : ", ");
        }
        free(edges);
    }
    else
    {
        printf("Nenhuma aresta (ou erro ao obter arestas).");
    }
    printf("\n");

    printf("Tipo: %s\n", g->directed ? "Direcionado" : "Não Direcionado");
    printf("---------------------------\n");
}

/**
 * @brief Lê um arquivo e cria um grafo a partir dele.
 *        O arquivo deve conter o número de arestas na primeira linha
 *        e, nas linhas seguintes, pares de inteiros representando as arestas.
 *
 * @param[in]  file_path Caminho do arquivo de entrada (não pode ser NULL).
 * @param[in]  directed  true para grafo dirigido, false para não dirigido.
 * @return Ponteiro para o grafo criado, ou NULL em caso de erro.
 *
 * @note O arquivo deve ter o formato:
 *       <num_arestas>
 *       <v1>, <v2>
 *       <v1>, <v2>
 *       ...
 * @note O usuário é responsável por liberar o grafo criado.
 * @note CERT C: API00-C, FIO34-C, MEM35-C, ERR33-C, DCL13-C, INT30-C
 */
Graph *graph_from_file(const char *file_path, bool directed)
{
    // Validação de parâmetros (API00-C, DCL13-C)
    if (file_path == NULL)
    {
        fprintf(stderr, "[ERRO] graph_from_file: Caminho do arquivo é NULL.\n");
        return NULL;
    }

    FILE *file = fopen(file_path, "r");
    if (!file)
    {
        fprintf(stderr, "[ERRO] graph_from_file: Erro ao abrir arquivo '%s'.\n", file_path);
        return NULL;
    }

    // Lê o número de arestas
    int num_edges = 0;
    {
        char line[256];
        if (!fgets(line, sizeof(line), file))
        {
            fprintf(stderr, "[ERRO] graph_from_file: Erro ao ler o número de arestas.\n");
            fclose(file);
            return NULL;
        }
        if (sscanf(line, "%d", &num_edges) != 1 || num_edges <= 0)
        {
            fprintf(stderr, "[ERRO] graph_from_file: Número de arestas inválido.\n");
            fclose(file);
            return NULL;
        }
    }

    // Prevenção de overflow (MEM35-C, INT30-C)
    if ((size_t)num_edges > SIZE_MAX / sizeof(int))
    {
        fprintf(stderr, "[ERRO] graph_from_file: Número de arestas muito grande (overflow).\n");
        fclose(file);
        return NULL;
    }

    int *edges_a = (int *)safe_malloc((size_t)num_edges * sizeof(int));
    int *edges_b = (int *)safe_malloc((size_t)num_edges * sizeof(int));
    if (!edges_a || !edges_b)
    {
        fprintf(stderr, "[ERRO] graph_from_file: Falha ao alocar memória para arestas.\n");
        if (edges_a)
            free(edges_a);
        if (edges_b)
            free(edges_b);
        fclose(file);
        return NULL;
    }

    int max_vertex = -1;
    for (int i = 0; i < num_edges; i++)
    {
        char line[256];
        if (!fgets(line, sizeof(line), file))
        {
            fprintf(stderr, "[ERRO] graph_from_file: Erro ao ler a aresta %d.\n", i + 1);
            free(edges_a);
            free(edges_b);
            fclose(file);
            return NULL;
        }

        int a, b;
        if (sscanf(line, "%d, %d", &a, &b) != 2)
        {
            fprintf(stderr, "[ERRO] graph_from_file: Formato de aresta inválido na linha %d.\n", i + 2);
            free(edges_a);
            free(edges_b);
            fclose(file);
            return NULL;
        }

        if (a < 0 || b < 0)
        {
            fprintf(stderr, "[ERRO] graph_from_file: Vértice negativo na linha %d: (%d, %d).\n", i + 2, a, b);
            free(edges_a);
            free(edges_b);
            fclose(file);
            return NULL;
        }

        edges_a[i] = a;
        edges_b[i] = b;

        if (a > max_vertex)
            max_vertex = a;
        if (b > max_vertex)
            max_vertex = b;
    }

    fclose(file);

    // Prevenção de overflow ao criar o grafo (MEM35-C, INT30-C)
    if (max_vertex < 0 || (size_t)max_vertex >= SIZE_MAX)
    {
        fprintf(stderr, "[ERRO] graph_from_file: Índice de vértice fora do intervalo suportado.\n");
        free(edges_a);
        free(edges_b);
        return NULL;
    }

    Graph *g = create_graph((size_t)max_vertex + 1, directed);
    if (!g)
    {
        fprintf(stderr, "[ERRO] graph_from_file: Falha ao criar o grafo.\n");
        free(edges_a);
        free(edges_b);
        return NULL;
    }

    // Adiciona as arestas
    for (int i = 0; i < num_edges; i++)
    {
        int a = edges_a[i];
        int b = edges_b[i];

        if (a < 0 || b < 0 || a > max_vertex || b > max_vertex)
        {
            fprintf(stderr, "[AVISO] graph_from_file: Aresta (%d, %d) fora do intervalo esperado. Ignorando.\n", a, b);
            continue;
        }
        if (add_edge(g, (size_t)a, (size_t)b) != 0)
        {
            fprintf(stderr, "[AVISO] graph_from_file: Falha ao adicionar aresta (%d, %d).\n", a, b);
            // Continua tentando adicionar as demais arestas
        }
    }

    free(edges_a);
    free(edges_b);
    return g;
}

/**
 * @brief Libera toda a memória alocada dinamicamente para a estrutura do grafo.
 *
 * Esta função libera, de forma segura e robusta, todos os recursos associados a um grafo,
 * incluindo as listas de adjacência de cada vértice, o vetor de ponteiros para as listas,
 * o vetor de tamanhos das listas e a própria estrutura do grafo.
 *
 * @param[in,out] g Ponteiro para o grafo a ser liberado. Após a chamada, o ponteiro
 *                  'g' não deve ser mais utilizado, pois a memória para a qual ele
 *                  aponta será inválida.
 * @pre g pode ser NULL (a função trata isso graciosamente).
 * @post Toda a memória associada ao grafo 'g' é liberada.
 * @note Esta função é idempotente: chamar free_graph(NULL) ou chamar free_graph
 *       em um grafo já liberado (se o ponteiro for NULL) é seguro.
 * @note Após a chamada, o ponteiro passado não é modificado (passagem por valor).
 * @note CERT C: MEM31-C, MEM30-C, MEM35-C, API00-C, DCL13-C, ERR33-C
 */
void free_graph(Graph *g)
{
    if (g == NULL)
    {
        // Função idempotente para ponteiros nulos
        return;
    }

    // Libera listas de adjacência de cada vértice, se existirem
    if (g->adj != NULL)
    {
        for (size_t i = 0; i < g->num_vertices; i++)
        {
            if (g->adj[i] != NULL)
            {
                free(g->adj[i]);
                g->adj[i] = NULL; // Previne double-free se a estrutura sobreviver
            }
        }
        free(g->adj);
        g->adj = NULL;
    }

    // Libera vetor de tamanhos das listas de adjacência
    if (g->adj_size != NULL)
    {
        free(g->adj_size);
        g->adj_size = NULL;
    }

    // Libera a própria estrutura do grafo
    free(g);
    // Não é possível modificar o ponteiro do chamador (passagem por valor)
}

/**
 * @brief Calcula o menor caminho entre dois vértices usando Dijkstra (pesos unitários).
 *
 * @param[in]  g         Ponteiro para o grafo (não pode ser NULL).
 * @param[in]  source    Vértice de origem (0 <= source < g->num_vertices).
 * @param[in]  target    Vértice de destino (0 <= target < g->num_vertices).
 * @param[out] out_path  (Opcional) Se não NULL, recebe um array dinâmico com o caminho (source...target).
 *                       O usuário é responsável por liberar o array retornado.
 * @param[out] out_len   (Opcional) Se não NULL, recebe o número de vértices do caminho (incluindo source e target).
 * @return Distância mínima entre source e target, ou INT_MAX se não há caminho.
 *
 * @note Para grafos ponderados, adapte a estrutura do grafo para armazenar pesos.
 * @note Regras CERT C: API00-C, ARR30-C, MEM35-C, ERR33-C, DCL13-C, INT30-C
 */
int dijkstra_shortest_path(const Graph *g, size_t source, size_t target, int **out_path, size_t *out_len)
{
    if (!g || !g->adj || !g->adj_size)
    {
        fprintf(stderr, "[ERRO] dijkstra_shortest_path: Grafo ou membros internos são NULL.\n");
        if (out_path)
            *out_path = NULL;
        if (out_len)
            *out_len = 0;
        return INT_MAX;
    }
    if (source >= g->num_vertices || target >= g->num_vertices)
    {
        fprintf(stderr, "[ERRO] dijkstra_shortest_path: Vértice fora do intervalo (source=%zu, target=%zu).\n", source, target);
        if (out_path)
            *out_path = NULL;
        if (out_len)
            *out_len = 0;
        return INT_MAX;
    }
    if (g->num_vertices == 0 || g->num_vertices > SIZE_MAX / sizeof(int))
    {
        fprintf(stderr, "[ERRO] dijkstra_shortest_path: Número de vértices inválido ou overflow.\n");
        if (out_path)
            *out_path = NULL;
        if (out_len)
            *out_len = 0;
        return INT_MAX;
    }

    size_t n = g->num_vertices;
    int *dist = (int *)safe_malloc(n * sizeof(int));
    bool *visited = (bool *)safe_calloc(n, sizeof(bool));
    size_t *prev = (size_t *)safe_malloc(n * sizeof(size_t)); // Para reconstruir o caminho
    if (!dist || !visited || !prev)
    {
        fprintf(stderr, "[ERRO] dijkstra_shortest_path: Falha ao alocar memória.\n");
        free(dist);
        free(visited);
        free(prev);
        if (out_path)
            *out_path = NULL;
        if (out_len)
            *out_len = 0;
        return INT_MAX;
    }

    for (size_t i = 0; i < n; i++)
    {
        dist[i] = INT_MAX;
        prev[i] = SIZE_MAX; // Sentinela para "sem predecessor"
    }
    dist[source] = 0;

    for (size_t count = 0; count < n; count++)
    {
        // Encontra o vértice não visitado com menor distância
        int min_dist = INT_MAX;
        size_t u = n;
        for (size_t v = 0; v < n; v++)
        {
            if (!visited[v] && dist[v] < min_dist)
            {
                min_dist = dist[v];
                u = v;
            }
        }
        if (u == n || dist[u] == INT_MAX)
            break; // Todos os alcançáveis já visitados

        visited[u] = true;
        if (u == target)
            break; // Chegou ao destino

        for (size_t i = 0; i < g->adj_size[u]; i++)
        {
            size_t v = g->adj[u][i];
            if (v >= n)
                continue;
            if (!visited[v] && dist[u] != INT_MAX && dist[u] + 1 < dist[v])
            {
                dist[v] = dist[u] + 1;
                prev[v] = u;
            }
        }
    }

    // Reconstrói o caminho, se solicitado
    if (out_path)
    {
        *out_path = NULL;
        if (dist[target] != INT_MAX)
        {
            // Caminho existe: reconstrói do target até o source
            size_t path_len = 1;
            for (size_t v = target; v != source; v = prev[v])
                path_len++;
            int *path = (int *)safe_malloc(path_len * sizeof(int));
            if (path)
            {
                size_t idx = path_len;
                size_t v = target;
                while (1)
                {
                    path[--idx] = (int)v;
                    if (v == source)
                        break;
                    v = prev[v];
                }
                *out_path = path;
                if (out_len)
                    *out_len = path_len;
            }
            else
            {
                if (out_len)
                    *out_len = 0;
            }
        }
        else
        {
            if (out_len)
                *out_len = 0;
        }
    }

    int result = dist[target];
    free(dist);
    free(visited);
    free(prev);
    return result;
}

/******************** FUNÇÃO PRINCIPAL (EXEMPLO) ********************/

/**
 * @brief Função principal de demonstração de operações com grafos.
 *
 * Lê um grafo de um arquivo, imprime sua estrutura, executa buscas BFS e DFS,
 * verifica a existência de ciclos e libera todos os recursos alocados.
 *
 * @return EXIT_SUCCESS em caso de sucesso, EXIT_FAILURE em caso de erro crítico.
 *
 * @note O arquivo "grafo.txt" deve estar no formato:
 *       <num_arestas>
 *       <v1>, <v2>
 *       <v1>, <v2>
 *       ...
 * @note Não utiliza ssize_t em nenhum momento.
 * @note Regras CERT C aplicadas: API00-C, ERR33-C, MEM31-C, DCL13-C, INT30-C
 */
int main(void)
{
    int exit_status = EXIT_SUCCESS;
    Graph *g = NULL;
    int *bfs_result = NULL;
    int *dfs_result = NULL;

    // Cria o grafo a partir do arquivo
    g = graph_from_file("./grafo.txt", false);
    if (g == NULL)
    {
        fprintf(stderr, "[ERRO] main: Grafo não pôde ser criado a partir do arquivo.\n");
        exit_status = EXIT_FAILURE;
        goto cleanup;
    }

    // Imprime informações do grafo
    print_graph(g);

    // Executa BFS a partir do vértice 0
    bfs_result = bfs(g, 0);
    if (bfs_result != NULL)
    {
        printf("BFS (iniciando em 0): ");
        for (size_t i = 0; i < g->num_vertices; i++)
        {
            printf("%d ", bfs_result[i]);
        }
        printf("\n");
    }
    else
    {
        fprintf(stderr, "[ERRO] main: Falha ao executar BFS.\n");
        exit_status = EXIT_FAILURE;
        goto cleanup;
    }

    // Executa DFS a partir do vértice 0
    dfs_result = dfs(g, 0);
    if (dfs_result != NULL)
    {
        printf("DFS (iniciando em 0): ");
        for (size_t i = 0; i < g->num_vertices; i++)
        {
            printf("%d ", dfs_result[i]);
        }
        printf("\n");
    }
    else
    {
        fprintf(stderr, "[ERRO] main: Falha ao executar DFS.\n");
        exit_status = EXIT_FAILURE;
        goto cleanup;
    }

    // Verifica se o grafo possui ciclo
    printf("Has cycle: %s\n", has_cycle(g) ? "Yes" : "No");

cleanup:
    // Libera recursos alocados
    if (bfs_result != NULL)
    {
        free(bfs_result);
        bfs_result = NULL;
    }
    if (dfs_result != NULL)
    {
        free(dfs_result);
        dfs_result = NULL;
    }
    if (g != NULL)
    {
        free_graph(g);
        g = NULL;
    }
    return exit_status;
}