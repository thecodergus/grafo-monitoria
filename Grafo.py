

import re


class Grafo:
    grafo = {}

    def __init__(self, arquivo = "", dirigido = False):
        self.dirigido = dirigido
        self.enderecoArquivo = arquivo

    def addAresta(self, vertice, vizinho):
        if vertice not in self.grafo:
            self.grafo[vertice] = [vizinho]
        elif vizinho not in self.grafo[vertice]:
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

    def temCiclo(self) -> bool:
        vertices = self.getAllVertices()

        

        return False

        

if __name__ == "__main__":
    pass