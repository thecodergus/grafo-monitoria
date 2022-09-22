from collections import defaultdict

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

    def getAllVertices(self) -> list:
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
        vertices = self.getAllVertices()
        
        print("Vertices:")
        for i, s in enumerate(vertices):
            print(f'{s}', end = "")
            if i % 10 == 0:
                print()

        return self

    def showArestas(self) -> object:
        vertices = self.getAllVertices()
        i = 0
        for s1 in vertices:
            for s2 in self.grafo[s1]:
                print(f'({s1}, {s2})', end = ", ")
                i += 1
                if i % 10 == 0:
                    print()

        return self

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
    
    g.showArestas()