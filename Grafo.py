

from collections import defaultdict

class Grafo:
    grafo = defaultdict(list)

    def __init__(self, arquivo = "", dirigido = False):
        self.dirigido = dirigido
        self.enderecoArquivo = arquivo

    def addAresta(self, vertice, vizinho):
        if vizinho not in self.grafo[vertice]:
            self.grafo[vertice].append(vizinho)

    def rmAresta(self, vertice, vizinho):
        pass

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
        numVertices = len(self.getAllVertices())
        visitado = [False] * (numVertices)

        for i in range(numVertices):
            if not visitado[i]:
                if self.__temCiclo(i, visitado, -1):
                    return True

        return False

        

if __name__ == "__main__":
    pass