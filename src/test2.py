class Cell:
    def __init__(self, g, f, position, parentIndex) -> None:
        self.g = g
        self.f = f
        self.position = position
        self.parentIndex = parentIndex


startingPos = (0, 0)
endingPos = (41, 10)

matrixWidth = 42
matrixHeight = 30
matrix = [[0 for _ in range(matrixWidth)] for _ in range(matrixHeight)]

openList = []
closedList = []

openList.append(Cell(0, 0, startingPos, 0))

while len(openList) > 0:
    bestCellIndex = 0
    for index, item in enumerate(openList):
        if item.f < openList[bestCellIndex].f:
            bestCellIndex = index

    currentCell = openList[bestCellIndex]

    openList.pop(bestCellIndex)
    closedList.append(currentCell)

    for x in range(-1, 2):
        for y in range(-1, 2):
            if x == y == 0:
                continue

            childPos = (currentCell.position[0] + x, currentCell.position[1] + y)

            if childPos == endingPos:
                path = [childPos]
                nextIndex = len(closedList) - 1
                while True:
                    path.insert(0, closedList[nextIndex].position)
                    if closedList[nextIndex].position == startingPos:
                        break
                    nextIndex = closedList[nextIndex].parentIndex
                print(path)
                exit()

            if not 0 <= childPos[0] < matrixWidth or not 0 <= childPos[1] < matrixHeight:
                continue 

            if matrix[childPos[1]][childPos[0]] == 1:
                continue
            
            found = False
            for closedChild in closedList:
                if closedChild.position == childPos:
                    found = True
                    break
            
            if found:
                continue

            g = currentCell.g + (14 if x != 0 and y != 0 else 10)

            xDistance = abs(childPos[0] - endingPos[0])
            yDistance = abs(childPos[1] - endingPos[1])
            h = (xDistance * 10 + yDistance * 4) if xDistance > yDistance else (xDistance * 4 + yDistance * 10)
            f = g + h

            # Child is already in the open list
            found = False
            for openCell in openList:
                if openCell.position == childPos and g > openCell.g:
                    found = True
                    continue
            
            if found:
                continue

            # Add the child to the open list
            openList.append(Cell(g, f, childPos, len(closedList) - 1))
        
