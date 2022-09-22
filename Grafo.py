from collections import defaultdict
from turtle import color, width
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

    def getArestas(self) -> list:
        resultado, vertices = [], self.getVertices()

        for s1 in vertices:
            for s2 in self.grafo[s1]:
                resultado.append([s1, s2])

        return resultado

    def showMapa2D(self) -> None:
        Vertices, Arestas = self.getVertices(), self.getArestas()

        # congig
        interface = net.Network(
                height='100%', 
                width='100%',
                notebook=False, 
                heading='Grafo 2D'
            )
        
        # Add Vertices
        for v in Vertices:
            interface.add_node(v, label=str(v))

        # Add Arestas
        interface.add_edges(Arestas)

        # export interface
        interface.show_buttons(filter_=['physics'])
        interface.show('Grafo2D.html')
        display(HTML('Grafo2D.html'))


    def showMapa3D(self):
        numVertices, Arestas = self.getNumVertices(), self.getArestas()

        # Gerar posições aleatorias para os nodos num plano 3D
        locs = []
        for i in range(numVertices + 1):
            locs.append([
                random.random(),
                random.random(),
                random.random()
            ])

        # Configs
        x_vertices = [locs[i][0] for i in range(numVertices)]
        y_vertices = [locs[i][1] for i in range(numVertices)]
        z_vertices = [locs[i][2] for i in range(numVertices)]

        x_arestas, y_arestas, z_arestas = [], [], []

        for aresta in Arestas:
            x_arestas.append([
                locs[aresta[0]][0], locs[aresta[1]][0], None
            ])
            
            y_arestas.append([
                locs[aresta[0]][1], locs[aresta[1]][1], None
            ])

            z_arestas.append([
                locs[aresta[0]][2], locs[aresta[1]][2], None
            ])

        trace_arestas = go.Scatter3d(
            x = x_arestas,
            y = y_arestas,
            z = z_arestas,
            mode = "lines",
            line = dict(
                color = "black",
                width = 2
            ),
            hoverinfo="none"
        )

        trace_vertices = go.Scatter3d(
            x = x_vertices,
            y = y_vertices,
            z = z_vertices,
            mode = "markers",
            marker = dict(
                symbol = "circle",
                size = 10,
                color = "blue",
                line = dict(
                    color = "black",
                    width = 0.5
                )
            ),
            hoverinfo = "text"
        )

        axis = dict(
            showbackground=False,
            showline=False,
            zeroline=False,
            showgrid=False,
            showticklabels=False,
            title=''
        )

        layout = go.Layout(
            title = "Grafo 3D",
            width=650,
            height=625,
            showlegend=False,
            scene=dict(xaxis=dict(axis),
                    yaxis=dict(axis),
                    zaxis=dict(axis),
                    ),
            margin=dict(t=100),
            hovermode='closest'
        )

        data = [trace_arestas, trace_vertices]

        # Gerar figura 3D
        fig = go.Figure(data=data, layout=layout)
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
