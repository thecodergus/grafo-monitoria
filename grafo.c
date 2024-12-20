#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/*
 * Estrutura de dados para representar um grafo em C.
 * Utiliza listas de adjacência com vetores dinâmicos.
 * Assume vértices enumerados de 0 a num_vertices-1.
 */

typedef struct {
    int **adj;               // Listas de adjacência (cada posição um array dinâmico de vizinhos)
    size_t *adj_size;        // Vetor com o tamanho da lista de adjacência de cada vértice
    size_t num_vertices;     // Número de vértices
    bool directed;           // Indica se o grafo é direcionado
} Graph;


/******************** FUNÇÕES AUXILIARES DE MEMÓRIA ********************/

/*
 * Função auxiliar para verificar alocação.
 * Caso ptr seja NULL, imprime mensagem de erro e aborta o programa.
 */
static void *check_allocation(void *ptr) {
    if (ptr == NULL) {
        fprintf(stderr, "Erro de alocação de memória\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

/*
 * Função auxiliar para realocar memória com segurança.
 * Recebe o ponteiro, o novo tamanho, tenta realocar e verifica erro.
 */
static void *safe_realloc(void *ptr, size_t new_size) {
    void *new_ptr = realloc(ptr, new_size);
    if (new_ptr == NULL && new_size > 0) {
        fprintf(stderr, "Erro de realocação de memória\n");
        free(ptr);
        exit(EXIT_FAILURE);
    }
    return new_ptr;
}

/*
 * Função auxiliar para alocar memória com calloc e verificação.
 */
static void *safe_calloc(size_t count, size_t size) {
    void *ptr = calloc(count, size);
    return check_allocation(ptr);
}

/*
 * Função auxiliar para alocar memória com malloc e verificação.
 */
static void *safe_malloc(size_t size) {
    void *ptr = malloc(size);
    return check_allocation(ptr);
}


/******************** FUNÇÕES DO GRAFO ********************/

/*
 * Cria um grafo com num_vertices vértices.
 * Param: num_vertices (quantidade de vértices)
 * Param: directed (se o grafo é dirigido ou não)
 * Retorna: ponteiro para Graph
 */
Graph* create_graph(size_t num_vertices, bool directed) {
    Graph *g = (Graph*) safe_malloc(sizeof(Graph));
    g->num_vertices = num_vertices;
    g->directed = directed;

    g->adj = (int**) safe_malloc(num_vertices * sizeof(int*));
    g->adj_size = (size_t*) safe_calloc(num_vertices, sizeof(size_t));

    for (size_t i = 0; i < num_vertices; i++) {
        g->adj[i] = NULL;
        g->adj_size[i] = 0;
    }

    return g;
}

/*
 * Adiciona uma aresta entre v e w.
 * Para grafo não-direcionado, adiciona aresta tanto em v quanto em w.
 */
void add_edge(Graph *g, int v, int w) {
    if ((size_t)v >= g->num_vertices || (size_t)w >= g->num_vertices) {
        fprintf(stderr, "Índice de vértice inválido em add_edge.\n");
        return;
    }

    // Redimensiona a lista de adj do vértice v para adicionar w
    g->adj[v] = (int*) safe_realloc(g->adj[v], (g->adj_size[v] + 1)*sizeof(int));
    g->adj[v][g->adj_size[v]] = w;
    g->adj_size[v]++;

    // Se não for direcionado, adiciona também v em w
    if (!g->directed) {
        g->adj[w] = (int*) safe_realloc(g->adj[w], (g->adj_size[w] + 1)*sizeof(int));
        g->adj[w][g->adj_size[w]] = v;
        g->adj_size[w]++;
    }
}

/*
 * Remove a aresta entre v e w.
 * Para não direcionado, remove também a aresta (w, v).
 */
void remove_edge(Graph *g, int v, int w) {
    if ((size_t)v >= g->num_vertices || (size_t)w >= g->num_vertices) {
        fprintf(stderr, "Índice de vértice inválido em remove_edge.\n");
        return;
    }

    // Remove w da lista de adj de v
    for (size_t i = 0; i < g->adj_size[v]; i++) {
        if (g->adj[v][i] == w) {
            // Shift dos elementos após a remoção
            for (size_t j = i; j + 1 < g->adj_size[v]; j++) {
                g->adj[v][j] = g->adj[v][j+1];
            }
            g->adj_size[v]--;
            g->adj[v] = (int*) safe_realloc(g->adj[v], g->adj_size[v]*sizeof(int));
            break;
        }
    }

    // Se não for direcionado, remove v da lista de w
    if (!g->directed) {
        for (size_t i = 0; i < g->adj_size[w]; i++) {
            if (g->adj[w][i] == v) {
                for (size_t j = i; j + 1 < g->adj_size[w]; j++) {
                    g->adj[w][j] = g->adj[w][j+1];
                }
                g->adj_size[w]--;
                g->adj[w] = (int*) safe_realloc(g->adj[w], g->adj_size[w]*sizeof(int));
                break;
            }
        }
    }
}

/*
 * Retorna o número de vértices do grafo.
 */
size_t get_num_vertices(const Graph *g) {
    return g->num_vertices;
}

/*
 * Retorna o grau do vértice vertex.
 */
size_t get_degree(const Graph *g, int vertex) {
    if ((size_t)vertex >= g->num_vertices) {
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
int* bfs(const Graph *g, int start) {
    if ((size_t)start >= g->num_vertices) {
        fprintf(stderr, "Índice de vértice inválido em bfs.\n");
        return NULL;
    }

    bool *visited = (bool*) safe_calloc(g->num_vertices, sizeof(bool));
    int *result = (int*) safe_malloc(g->num_vertices * sizeof(int));
    int *queue = (int*) safe_malloc(g->num_vertices * sizeof(int));

    size_t front = 0, rear = 0;
    visited[start] = true;
    queue[rear++] = start;

    size_t count = 0;
    while (front < rear) {
        int v = queue[front++];
        result[count++] = v;

        for (size_t i = 0; i < g->adj_size[v]; i++) {
            int neigh = g->adj[v][i];
            if (!visited[neigh]) {
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
int* dfs(const Graph *g, int start) {
    if ((size_t)start >= g->num_vertices) {
        fprintf(stderr, "Índice de vértice inválido em dfs.\n");
        return NULL;
    }

    bool *visited = (bool*) safe_calloc(g->num_vertices, sizeof(bool));
    int *stack = (int*) safe_malloc(g->num_vertices * sizeof(int));
    int *path = (int*) safe_malloc(g->num_vertices * sizeof(int));

    ssize_t top = -1; // Utilize ssize_t para permitir -1
    stack[++top] = start;

    size_t count = 0;
    while (top >= 0) {
        int v = stack[top--];
        if (!visited[v]) {
            visited[v] = true;
            path[count++] = v;
            // Adiciona vizinhos na pilha (poderia inverter a ordem se desejado)
            for (size_t i = 0; i < g->adj_size[v]; i++) {
                int neigh = g->adj[v][i];
                if (!visited[neigh]) {
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
static bool has_cycle_util(const Graph *g, int v, bool visited[], int parent) {
    visited[v] = true;

    for (size_t i = 0; i < g->adj_size[v]; i++) {
        int neigh = g->adj[v][i];
        if (!visited[neigh]) {
            if (has_cycle_util(g, neigh, visited, v))
                return true;
        } else if (neigh != parent) {
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
bool has_cycle(const Graph *g) {
    bool *visited = (bool*) safe_calloc(g->num_vertices, sizeof(bool));

    for (size_t v = 0; v < g->num_vertices; v++) {
        if (!visited[v]) {
            if (has_cycle_util(g, (int)v, visited, -1)) {
                free(visited);
                return true;
            }
        }
    }

    free(visited);
    return false;
}

typedef struct {
    int v1, v2;
} Edge;

/*
 * Retorna a lista de arestas do grafo.
 * O usuário libera a memória do array retornado.
 * as_tuple (bool) não faz diferença tão grande aqui, mas mantemos por compatibilidade.
 */
Edge* get_edges(const Graph *g, bool as_tuple, size_t *num_edges) {
    *num_edges = 0;
    // Estimativa superdimensionada, no pior caso um grafo completo tem O(n²) arestas.
    // Ajustaremos depois com realloc se quiser.
    size_t capacity = g->num_vertices * g->num_vertices;
    Edge *edges = (Edge*) safe_malloc(capacity * sizeof(Edge));

    bool *processed = NULL;
    if (!g->directed) {
        processed = (bool*) safe_calloc(capacity, sizeof(bool));
    }

    for (size_t v = 0; v < g->num_vertices; v++) {
        for (size_t i = 0; i < g->adj_size[v]; i++) {
            int w = g->adj[v][i];
            if (g->directed) {
                // Grafo dirigido: adiciona a aresta (v, w)
                edges[*num_edges].v1 = (int)v;
                edges[*num_edges].v2 = w;
                (*num_edges)++;
            } else {
                // Grafo não dirigido: armazenamos a aresta ordenada
                int min = ((int)v < w) ? (int)v : w;
                int max = ((int)v < w) ? w : (int)v;
                size_t index = (size_t)min*g->num_vertices + (size_t)max;
                if (!processed[index]) {
                    edges[*num_edges].v1 = min;
                    edges[*num_edges].v2 = max;
                    processed[index] = true;
                    (*num_edges)++;
                }
            }
        }
    }

    // Redimensiona edges para o tamanho real
    edges = (Edge*) safe_realloc(edges, (*num_edges)*sizeof(Edge));
    if (processed) free(processed);
    return edges;
}

/*
 * Imprime informações sobre o grafo, similar ao __str__ do Python.
 */
void print_graph(const Graph *g) {
    printf("Vertices: ");
    for (size_t i = 0; i < g->num_vertices; i++) {
        printf("%zu ", i);
    }
    printf("\nNumber of Vertices: %zu\n", g->num_vertices);

    size_t num_edges;
    Edge* edges = get_edges(g, true, &num_edges);

    printf("Edges: ");
    for (size_t i = 0; i < num_edges; i++) {
        printf("(%d, %d) ", edges[i].v1, edges[i].v2);
    }
    printf("\nNumber of Edges: %zu\n", num_edges);

    free(edges);
}

/*
 * Lê um arquivo e cria um grafo a partir dele.
 * O arquivo deve conter arestas com dois inteiros por linha.
 */
Graph* graph_from_file(const char *file_path, bool directed) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        fprintf(stderr, "Erro ao abrir arquivo '%s'\n", file_path);
        return NULL;
    }

    // Lê o número de arestas
    int num_edges = 0;
    {
        char line[256];
        if (!fgets(line, sizeof(line), file)) {
            fprintf(stderr, "Erro ao ler o número de arestas.\n");
            fclose(file);
            return NULL;
        }
        if (sscanf(line, "%d", &num_edges) != 1 || num_edges <= 0) {
            fprintf(stderr, "Número de arestas inválido.\n");
            fclose(file);
            return NULL;
        }
    }

    // Agora sabemos quantas arestas esperar. Precisamos determinar o maior vértice.
    // Vamos primeiro armazenar as arestas em memória para depois criar o grafo do tamanho adequado.
    int *edges_a = (int*) safe_malloc(num_edges * sizeof(int));
    int *edges_b = (int*) safe_malloc(num_edges * sizeof(int));

    int max_vertex = -1;
    {
        for (int i = 0; i < num_edges; i++) {
            char line[256];
            if (!fgets(line, sizeof(line), file)) {
                fprintf(stderr, "Erro ao ler a aresta %d.\n", i+1);
                free(edges_a);
                free(edges_b);
                fclose(file);
                return NULL;
            }

            int a, b;
            // Note o formato "%d, %d" para ler algo como "1, 2"
            if (sscanf(line, "%d, %d", &a, &b) != 2) {
                fprintf(stderr, "Formato de aresta inválido na linha %d.\n", i+2); // +2: primeira linha é número de arestas
                free(edges_a);
                free(edges_b);
                fclose(file);
                return NULL;
            }

            edges_a[i] = a;
            edges_b[i] = b;

            if (a > max_vertex) max_vertex = a;
            if (b > max_vertex) max_vertex = b;
        }
    }

    // Volta ao início do arquivo não é mais necessário, pois já armazenamos tudo.
    fclose(file);

    // Agora cria o grafo com o tamanho max_vertex + 1
    Graph *g = create_graph((size_t)max_vertex + 1, directed);

    // Adiciona as arestas
    for (int i = 0; i < num_edges; i++) {
        int a = edges_a[i];
        int b = edges_b[i];

        if (a < 0 || b < 0 || a > max_vertex || b > max_vertex) {
            fprintf(stderr, "Aresta (%d, %d) fora do intervalo esperado.\n", a, b);
            // Não encerramos abruptamente, mas em um caso real poderíamos tratar isso melhor.
            // Aqui apenas continuamos. Se preferir, você pode encerrar o programa.
        } else {
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
void free_graph(Graph *g) {
    if (!g) return;
    for (size_t i = 0; i < g->num_vertices; i++) {
        free(g->adj[i]);
    }
    free(g->adj);
    free(g->adj_size);
    free(g);
}


/******************** FUNÇÃO PRINCIPAL (EXEMPLO) ********************/

int main(void) {
    // Exemplo de uso:
    // Supõe um arquivo "grafo.txt" com uma aresta por linha no formato:
    // a, b
    // ...
    Graph *g = graph_from_file("./grafo.txt", false);
    if (g == NULL) {
        fprintf(stderr, "Grafo não pôde ser criado a partir do arquivo.\n");
        return EXIT_FAILURE;
    }

    print_graph(g);

    int *bfs_result = bfs(g, 0);
    if (bfs_result) {
        printf("BFS (iniciando em 0): ");
        for (size_t i = 0; i < g->num_vertices; i++) {
            printf("%d ", bfs_result[i]);
        }
        printf("\n");
        free(bfs_result);
    }

    int *dfs_result = dfs(g, 0);
    if (dfs_result) {
        printf("DFS (iniciando em 0): ");
        for (size_t i = 0; i < g->num_vertices; i++) {
            printf("%d ", dfs_result[i]);
        }
        printf("\n");
        free(dfs_result);
    }

    printf("Has cycle: %s\n", has_cycle(g) ? "Yes" : "No");

    free_graph(g);
    return EXIT_SUCCESS;
}
