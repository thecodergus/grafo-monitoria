from collections import defaultdict
from pyvis import network as net
from IPython.display import display, HTML
import plotly.graph_objects as go
import random

class Grafo:
    grafo = defaultdict(list)

    def __init__(self, dirigido = False) -> None:
        self.dirigido = dirigido

    def addAresta(self, vertice, vizinho) -> object:
        if vizinho not in self.grafo[vertice]:
            self.grafo[vertice].append(vizinho)
            if not self.dirigido:
                self.grafo[vizinho].append(vertice)

        return self

    def rmAresta(self, vertice, vizinho) -> object:
        if vertice in self.grafo:
            self.grafo[vertice].remove(vizinho)
            if not self.dirigido:
                self.grafo[vizinho].remove(vertice)
        
        return self

    def getNumVertices(self) -> int:
        return len(self.grafo.keys())

    def getGrau(self, s) -> int:
        return len(self.grafo[s])

    def getVertices(self) -> list:
        return self.grafo.keys()

    def bfs(self, s) -> list:
        visitados, fila, resultado = [s], [s], []

        while fila:
            s = fila.pop(0)
            for vertice in self.grafo[s]:
                if vertice not in visitados:
                    visitados.append(vertice)
                    fila.append(vertice)
            resultado.append(s)

        return resultado

    def dfs(self, s) -> list:
        pilha, caminho = [s], []
        while pilha:
            vertice = pilha.pop()
            if vertice not in caminho:
                caminho.append(vertice)
                pilha.extend(self.grafo[vertice])
        
        return caminho

    def __temCiclo(self, s, visitado, parente) -> bool:
        visitado[s] = True

        for i in self.grafo[s]:
            if not visitado[i]:
                if self.__temCiclo(i, visitado, s):
                    return True
            elif parente != i:
                return True

        return False

    def temCiclo(self) -> bool:
        numVertices: int = self.getNumVertices()
        visitado: list = [False] * (numVertices + 1)

        for i in range(numVertices):
            if not visitado[i]:
                if self.__temCiclo(i, visitado, -1):
                    return True

        return False

    def showVertices(self) -> object:
        vertices = self.getVertices()
        
        print("Vertices:")
        for i, s in enumerate(vertices):
            print(f'{s}', end = "")
            if i % 10 == 0:
                print()

        return self

    def showArestas(self) -> object:
        vertices = self.getVertices()
        i = 0
        for s1 in vertices:
            for s2 in self.grafo[s1]:
                print(f'({s1}, {s2})', end = ", ")
                i += 1
                if i % 10 == 0:
                    print()

        return self

    def getArestas(self, tupla = False) -> list:
        resultado, vertices = [], self.getVertices()

        for s1 in vertices:
            for s2 in self.grafo[s1]:
                if tupla:
                    resultado.append((s1, s2))
                else:
                    resultado.append([s1, s2])

        return resultado

    def showMapa2D(self) -> None:
        Vertices, Arestas = self.getVertices(), self.getArestas()

        # congig
        interface = net.Network(
                height='100%', 
                width='100%',
                notebook=True, 
                heading='Grafo 2D'
            )
        
        # Add Vertices
        for v in Vertices:
            interface.add_node(v, label=str(v))

        # Add Arestas
        interface.add_edges(Arestas)

        # export interfaces
        interface.show('Grafo2D.html')
        display(HTML('Grafo2D.html'))


    def showMapa3D(self):
        numVertices = self.getNumVertices()
        edge_weights = [1] * (numVertices)
        edges = self.getArestas(tupla=True)

        spring_3D = defaultdict(list)

        random.SystemRandom()
        for i in range(1, numVertices + 1):
            # random.seed(i)
            spring_3D[i].extend([
                random.random(),
                random.random(),
                random.random()
            ])

        x_nodes = [spring_3D[key][0] for key in spring_3D.keys()]
        y_nodes = [spring_3D[key][1] for key in spring_3D.keys()]
        z_nodes = [spring_3D[key][2] for key in spring_3D.keys()]

        x_edges, y_edges, z_edges = [], [], []
        xtp, ytp, ztp = [], [], []

        for edge in edges:
            #formato: [comeco, destino, None]
            x_coords = [spring_3D[edge[0]][0], spring_3D[edge[1]][0], None]
            x_edges += x_coords
            xtp.append(0.5 * (spring_3D[edge[0]][0] + spring_3D[edge[1]][0]))

            y_coords = [spring_3D[edge[0]][1], spring_3D[edge[1]][1], None]
            y_edges += y_coords
            ytp.append(0.5 * (spring_3D[edge[0]][1] + spring_3D[edge[1]][1]))

            z_coords = [spring_3D[edge[0]][2], spring_3D[edge[1]][2], None]
            z_edges += z_coords
            ztp.append(0.5 * (spring_3D[edge[0]][2] + spring_3D[edge[1]][2]))


        etext = [f'weight={w}' for w in edge_weights]

        trace_weights = go.Scatter3d(
                x=xtp,
                y=ytp,
                z=ztp,
                mode='markers',
                # set the same color as for the edge lines
                marker=dict(color='rgb(125,125,125)', size=1),
                text=etext, 
                hoverinfo='text'
            )

        #create a trace for the edges
        trace_edges = go.Scatter3d(
                x=x_edges,
                y=y_edges,
                z=z_edges,
                mode='lines',
                line=dict(color='black', width=2),
                hoverinfo='none'
            )

        #create a trace for the nodes
        trace_nodes = go.Scatter3d(
            x=x_nodes,
            y=y_nodes,
            z=z_nodes,
            mode='markers',
            marker=dict(symbol='circle',
                        size=10,
                        color='skyblue')
        )

        #Include the traces we want to plot and create a figure
        data = [trace_edges, trace_nodes, trace_weights]
        fig = go.Figure(data=data)

        fig.show()




def GrafoFromFile(enderecoArquivo = "", dirigido = False) -> Grafo:
    grafo = Grafo(dirigido)
    split = lambda string: [*map(int, string.split(" "))]

    with open(enderecoArquivo, "r") as arquivo:
        for linha in arquivo.readlines():
            aresta = split(linha)
            if len(aresta) > 1:
                a, b = aresta
                grafo.addAresta(a, b)

    return grafo
            
        

if __name__ == "__main__":
    g = GrafoFromFile("teste.txt")
    
    # g.showMapa2D()
    g.showMapa3D()
