#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

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

/*
 * Retorna o número de vértices do grafo.
 */
size_t get_num_vertices(const Graph *g)
{
    return g->num_vertices;
}

/*
 * Retorna o grau do vértice vertex.
 */
size_t get_degree(const Graph *g, int vertex)
{
    if ((size_t)vertex >= g->num_vertices)
    {
        fprintf(stderr, "Índice de vértice inválido em get_degree.\n");
        return 0;
    }
    return g->adj_size[vertex];
}

/*
 * Executa BFS a partir de um vértice start.
 * Retorna um array dinâmico com a ordem de visita.
 * O usuário é responsável por liberar a memória retornada.
 */
int *bfs(const Graph *g, int start)
{
    if ((size_t)start >= g->num_vertices)
    {
        fprintf(stderr, "Índice de vértice inválido em bfs.\n");
        return NULL;
    }

    bool *visited = (bool *)safe_calloc(g->num_vertices, sizeof(bool));
    int *result = (int *)safe_malloc(g->num_vertices * sizeof(int));
    int *queue = (int *)safe_malloc(g->num_vertices * sizeof(int));

    size_t front = 0, rear = 0;
    visited[start] = true;
    queue[rear++] = start;

    size_t count = 0;
    while (front < rear)
    {
        int v = queue[front++];
        result[count++] = v;

        for (size_t i = 0; i < g->adj_size[v]; i++)
        {
            int neigh = g->adj[v][i];
            if (!visited[neigh])
            {
                visited[neigh] = true;
                queue[rear++] = neigh;
            }
        }
    }

    free(queue);
    free(visited);

    // count contém o número de vértices visitados
    // Nesse caso assume-se que todos podem ser visitados a partir de start.
    // Caso contrário, result conterá apenas os visitados.
    // O cliente pode utilizar count para saber quantos foram visitados.
    return result;
}

/*
 * Executa DFS iterativa a partir de start.
 * Retorna um array dinâmico com a ordem de visita.
 * O usuário libera a memória resultante.
 */
int *dfs(const Graph *g, int start)
{
    if ((size_t)start >= g->num_vertices)
    {
        fprintf(stderr, "Índice de vértice inválido em dfs.\n");
        return NULL;
    }

    bool *visited = (bool *)safe_calloc(g->num_vertices, sizeof(bool));
    int *stack = (int *)safe_malloc(g->num_vertices * sizeof(int));
    int *path = (int *)safe_malloc(g->num_vertices * sizeof(int));

    ssize_t top = -1; // Utilize ssize_t para permitir -1
    stack[++top] = start;

    size_t count = 0;
    while (top >= 0)
    {
        int v = stack[top--];
        if (!visited[v])
        {
            visited[v] = true;
            path[count++] = v;
            // Adiciona vizinhos na pilha (poderia inverter a ordem se desejado)
            for (size_t i = 0; i < g->adj_size[v]; i++)
            {
                int neigh = g->adj[v][i];
                if (!visited[neigh])
                {
                    stack[++top] = neigh;
                }
            }
        }
    }

    free(stack);
    free(visited);

    return path;
}

/*
 * Função auxiliar para detecção de ciclo em grafos não direcionados.
 * parent = -1 para o primeiro chamado.
 */
static bool has_cycle_util(const Graph *g, int v, bool visited[], int parent)
{
    visited[v] = true;

    for (size_t i = 0; i < g->adj_size[v]; i++)
    {
        int neigh = g->adj[v][i];
        if (!visited[neigh])
        {
            if (has_cycle_util(g, neigh, visited, v))
                return true;
        }
        else if (neigh != parent)
        {
            // Se o vizinho já foi visitado e não é o pai, há ciclo
            return true;
        }
    }
    return false;
}

/*
 * Verifica se o grafo possui ciclo.
 * Esta detecção está correta apenas para grafos não-direcionados.
 * Para grafos direcionados, seria necessário outro algoritmo (ex: DFS com pilha de recursão).
 */
bool has_cycle(const Graph *g)
{
    bool *visited = (bool *)safe_calloc(g->num_vertices, sizeof(bool));

    for (size_t v = 0; v < g->num_vertices; v++)
    {
        if (!visited[v])
        {
            if (has_cycle_util(g, (int)v, visited, -1))
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

/*
 * Retorna a lista de arestas do grafo.
 * O usuário libera a memória do array retornado.
 * as_tuple (bool) não faz diferença tão grande aqui, mas mantemos por compatibilidade.
 */
Edge *get_edges(const Graph *g, bool as_tuple, size_t *num_edges)
{
    *num_edges = 0;
    // Estimativa superdimensionada, no pior caso um grafo completo tem O(n²) arestas.
    // Ajustaremos depois com realloc se quiser.
    size_t capacity = g->num_vertices * g->num_vertices;
    Edge *edges = (Edge *)safe_malloc(capacity * sizeof(Edge));

    bool *processed = NULL;
    if (!g->directed)
    {
        processed = (bool *)safe_calloc(capacity, sizeof(bool));
    }

    for (size_t v = 0; v < g->num_vertices; v++)
    {
        for (size_t i = 0; i < g->adj_size[v]; i++)
        {
            int w = g->adj[v][i];
            if (g->directed)
            {
                // Grafo dirigido: adiciona a aresta (v, w)
                edges[*num_edges].v1 = (int)v;
                edges[*num_edges].v2 = w;
                (*num_edges)++;
            }
            else
            {
                // Grafo não dirigido: armazenamos a aresta ordenada
                int min = ((int)v < w) ? (int)v : w;
                int max = ((int)v < w) ? w : (int)v;
                size_t index = (size_t)min * g->num_vertices + (size_t)max;
                if (!processed[index])
                {
                    edges[*num_edges].v1 = min;
                    edges[*num_edges].v2 = max;
                    processed[index] = true;
                    (*num_edges)++;
                }
            }
        }
    }

    // Redimensiona edges para o tamanho real
    edges = (Edge *)safe_realloc(edges, (*num_edges) * sizeof(Edge));
    if (processed)
        free(processed);
    return edges;
}

/*
 * Imprime informações sobre o grafo, similar ao __str__ do Python.
 */
void print_graph(const Graph *g)
{
    printf("Vertices: ");
    for (size_t i = 0; i < g->num_vertices; i++)
    {
        printf("%zu ", i);
    }
    printf("\nNumber of Vertices: %zu\n", g->num_vertices);

    size_t num_edges;
    Edge *edges = get_edges(g, true, &num_edges);

    printf("Edges: ");
    for (size_t i = 0; i < num_edges; i++)
    {
        printf("(%d, %d) ", edges[i].v1, edges[i].v2);
    }
    printf("\nNumber of Edges: %zu\n", num_edges);

    free(edges);
}

/*
 * Lê um arquivo e cria um grafo a partir dele.
 * O arquivo deve conter arestas com dois inteiros por linha.
 */
Graph *graph_from_file(const char *file_path, bool directed)
{
    FILE *file = fopen(file_path, "r");
    if (!file)
    {
        fprintf(stderr, "Erro ao abrir arquivo '%s'\n", file_path);
        return NULL;
    }

    // Lê o número de arestas
    int num_edges = 0;
    {
        char line[256];
        if (!fgets(line, sizeof(line), file))
        {
            fprintf(stderr, "Erro ao ler o número de arestas.\n");
            fclose(file);
            return NULL;
        }
        if (sscanf(line, "%d", &num_edges) != 1 || num_edges <= 0)
        {
            fprintf(stderr, "Número de arestas inválido.\n");
            fclose(file);
            return NULL;
        }
    }

    // Agora sabemos quantas arestas esperar. Precisamos determinar o maior vértice.
    // Vamos primeiro armazenar as arestas em memória para depois criar o grafo do tamanho adequado.
    int *edges_a = (int *)safe_malloc(num_edges * sizeof(int));
    int *edges_b = (int *)safe_malloc(num_edges * sizeof(int));

    int max_vertex = -1;
    {
        for (int i = 0; i < num_edges; i++)
        {
            char line[256];
            if (!fgets(line, sizeof(line), file))
            {
                fprintf(stderr, "Erro ao ler a aresta %d.\n", i + 1);
                free(edges_a);
                free(edges_b);
                fclose(file);
                return NULL;
            }

            int a, b;
            // Note o formato "%d, %d" para ler algo como "1, 2"
            if (sscanf(line, "%d, %d", &a, &b) != 2)
            {
                fprintf(stderr, "Formato de aresta inválido na linha %d.\n", i + 2); // +2: primeira linha é número de arestas
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
    }

    // Volta ao início do arquivo não é mais necessário, pois já armazenamos tudo.
    fclose(file);

    // Agora cria o grafo com o tamanho max_vertex + 1
    Graph *g = create_graph((size_t)max_vertex + 1, directed);

    // Adiciona as arestas
    for (int i = 0; i < num_edges; i++)
    {
        int a = edges_a[i];
        int b = edges_b[i];

        if (a < 0 || b < 0 || a > max_vertex || b > max_vertex)
        {
            fprintf(stderr, "Aresta (%d, %d) fora do intervalo esperado.\n", a, b);
            // Não encerramos abruptamente, mas em um caso real poderíamos tratar isso melhor.
            // Aqui apenas continuamos. Se preferir, você pode encerrar o programa.
        }
        else
        {
            add_edge(g, a, b);
        }
    }

    free(edges_a);
    free(edges_b);
    return g;
}

/*
 * Libera a memória alocada para o grafo.
 */
void free_graph(Graph *g)
{
    if (!g)
        return;
    for (size_t i = 0; i < g->num_vertices; i++)
    {
        free(g->adj[i]);
    }
    free(g->adj);
    free(g->adj_size);
    free(g);
}

/******************** FUNÇÃO PRINCIPAL (EXEMPLO) ********************/

int main(void)
{
    // Exemplo de uso:
    // Supõe um arquivo "grafo.txt" com uma aresta por linha no formato:
    // a, b
    // ...
    Graph *g = graph_from_file("./grafo.txt", false);
    if (g == NULL)
    {
        fprintf(stderr, "Grafo não pôde ser criado a partir do arquivo.\n");
        return EXIT_FAILURE;
    }

    print_graph(g);

    int *bfs_result = bfs(g, 0);
    if (bfs_result)
    {
        printf("BFS (iniciando em 0): ");
        for (size_t i = 0; i < g->num_vertices; i++)
        {
            printf("%d ", bfs_result[i]);
        }
        printf("\n");
        free(bfs_result);
    }

    int *dfs_result = dfs(g, 0);
    if (dfs_result)
    {
        printf("DFS (iniciando em 0): ");
        for (size_t i = 0; i < g->num_vertices; i++)
        {
            printf("%d ", dfs_result[i]);
        }
        printf("\n");
        free(dfs_result);
    }

    printf("Has cycle: %s\n", has_cycle(g) ? "Yes" : "No");

    free_graph(g);
    return EXIT_SUCCESS;
}
