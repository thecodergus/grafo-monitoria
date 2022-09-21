from collections import defaultdict

class Grafo:
    grafo = defaultdict(list)

    def __init__(self, dirigido = False):
        self.dirigido = dirigido

    def addAresta(self, vertice, vizinho):
        if vizinho not in self.grafo[vertice]:
            self.grafo[vertice].append(vizinho)
            if not self.dirigido:
                self.grafo[vizinho].append(vertice)

        return self

    def rmAresta(self, vertice, vizinho):
        if vertice in self.grafo:
            self.grafo[vertice].remove(vizinho)
            if not self.dirigido:
                self.grafo[vizinho].remove(vertice)
        
        return self

    def getNumVertices(self) -> int:
        return len(self.grafo.keys())

    def getGrau(self, s) -> int:
        return len(self.grafo[s])

    def getAllVertices(self):
        return self.grafo.keys()

    def bfs(self, s):
        visitados, fila, resultado = [s], [s], []

        while fila:
            s = fila.pop(0)
            for vertice in self.grafo[s]:
                if vertice not in visitados:
                    visitados.append(vertice)
                    fila.append(vertice)
            resultado.append(s)

        return resultado

    def dfs(self, s):
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

    def showVertices(self):
        vertices = self.getAllVertices()

        for s in vertices:
            print(f'{s}')

    def showArestas(self):
        vertices = self.getAllVertices()
        print(vertices)
        for s1 in vertices:
            for s2 in self.grafo[s1]:
                print(f'({s1}, {s2})')

        

if __name__ == "__main__":
    g = Grafo()
    g.addAresta(1, 2).addAresta(2, 3).addAresta(2, 4).addAresta(4, 1)
    print(g.temCiclo())