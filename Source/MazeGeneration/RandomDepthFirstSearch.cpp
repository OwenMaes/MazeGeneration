// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomDepthFirstSearch.h"
#include "MazeGenerator.h"

RandomDepthFirstSearch::RandomDepthFirstSearch(FVector mazeStartPosition, int nrOfMazeColumns, int nrOfMazeRows, float MazeTileSize, 
	TArray<FMazeNode*>& arrayOfNodes, TArray<FMazeNodeConnection*>& arrayOfWalls)
	:MazeStartPosition(mazeStartPosition)
	,NrOfMazeColumns(nrOfMazeColumns)
	,NrOfMazeRows(nrOfMazeRows)
	,MazeTileSize(MazeTileSize )
	,ArrayOfNodes(arrayOfNodes)
	,ArrayOfWalls(arrayOfWalls)
{
	
}

RandomDepthFirstSearch::~RandomDepthFirstSearch()
{
}

void RandomDepthFirstSearch::DoWork()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Generating maze with DFS..."));

	ArrayOfNodes.Empty();
	ArrayOfWalls.Empty();

	CreateMazeGrid(ArrayOfWalls);
	CarvePath();

	//while (!CheckIfMazeIsValid())
	//{
	//	ResetMazeNodes();
	//	CarveMaze();
	//}

	//Create array for OUT array parameter
	MazeNodeGrid.GenerateValueArray(ArrayOfNodes);
}

void RandomDepthFirstSearch::CreateMazeGrid(TArray<FMazeNodeConnection*>& arrayOfWalls)
{
	//Create Maze Nodes and its adjacent connections (walls)
	TArray<FIntVector> MazeDirections = { { 1, 0, 0 }, { 0, 1, 0 }, { -1, 0, 0 }, { 0, -1, 0 } };
	FVector position{};
	int adjacentNodeIdx{}, adjacentRow{}, adjacentCol{};

	for (size_t row = 0; row < NrOfMazeRows; row++)
	{
		for (size_t col = 0; col < NrOfMazeColumns; col++)
		{
			//Create Maze node
			FMazeNode* newMazeNode = new FMazeNode();
			position = MazeStartPosition;
			position.X += MazeTileSize * col - MazeTileSize / 2;
			position.Y -= MazeTileSize * row + MazeTileSize / 2;
			newMazeNode->MazeNodeId = row * NrOfMazeColumns + col;
			newMazeNode->NodePosition = position;

			//Add connections (walls) in all valid directions
			for (auto& dir : MazeDirections)
			{
				adjacentCol = (col + dir.X);
				adjacentRow = (row + dir.Y);

				//Check if the directions in the grid are valid
				if (0 <= adjacentCol && adjacentCol < NrOfMazeColumns
					&& 0 <= adjacentRow && adjacentRow < NrOfMazeRows)
				{
					FMazeNodeConnection* newMazeConnection;
					adjacentNodeIdx = adjacentRow * NrOfMazeColumns + adjacentCol;
					newMazeConnection = new FMazeNodeConnection();
					newMazeConnection->FromNodeID = newMazeNode->MazeNodeId;
					newMazeConnection->FromNode = newMazeNode;
					newMazeConnection->ToNodeID = adjacentNodeIdx;
					newMazeNode->Connections.Add(newMazeConnection);
					arrayOfWalls.Add(newMazeConnection);
				}
				
			}
			MazeNodeGrid.Add(newMazeNode->MazeNodeId, newMazeNode);
		}
	}
}

void RandomDepthFirstSearch::CarvePath()
{
	int startNodeIDx = 0;
	TArray<FMazeNode*> backtrackPath{};
	if (auto startNode = *MazeNodeGrid.Find(startNodeIDx))
		RandomDFS(startNode, backtrackPath);
}

void RandomDepthFirstSearch::RandomDFS(FMazeNode* node, TArray<FMazeNode*>& nodePath)
{
	//The node is marked visited
	node->IsVisited = true;

	//Shuffle the connection array
	auto nrOfWalls = node->Connections.Num() - 1;
	int swapIdx{};
	for (auto i = 0; i < nrOfWalls; ++i) {
		swapIdx = FMath::RandRange(i+1, nrOfWalls);
		node->Connections.Swap(i, swapIdx);
	}

	//Choose a random wall that goes to a node not visited yet
	for (auto i = 0; i < nrOfWalls; i++)
	{

		FMazeNode* unvisitedNode = *MazeNodeGrid.Find(node->Connections[i]->ToNodeID);
		if (unvisitedNode)
		{
			node->Connections[i]->ToNode = unvisitedNode;
			if (unvisitedNode->IsVisited)
				continue;

			//Remove the wall of the connection
			node->Connections[i]->IsWall = false;
			if (auto adjacentNode = *MazeNodeGrid.Find(node->Connections[i]->ToNodeID)) {
				for (auto& conn : adjacentNode->Connections)
				{
					if (conn->ToNodeID == node->Connections[i]->FromNodeID)
					{
						conn->IsWall = false;
						break;
					}
				}
			}

			//Add current node to node path for backtracking
			nodePath.Add(node);

			//Go to unvisided node
			RandomDFS(unvisitedNode, nodePath);
		}
	}

	//Check if back at start node
	if (nodePath.Num() == 0)
		return;

	//Go back to node with unvisited connections
	auto prevNode = nodePath.Pop();
	RandomDFS(prevNode, nodePath);

}
