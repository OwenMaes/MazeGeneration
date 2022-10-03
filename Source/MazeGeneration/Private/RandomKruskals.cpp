// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomKruskals.h"
#include "../MazeGenerator.h"

RandomKruskals::RandomKruskals(FVector mazeStartPosition, int nrOfMazeColumns, int nrOfMazeRows, float MazeTileSize, TArray<FMazeNode*>& arrayOfNodes, TArray<FMazeNodeConnection*>& arrayOfWalls)
	:MazeStartPosition(mazeStartPosition)
	, NrOfMazeColumns(nrOfMazeColumns)
	, NrOfMazeRows(nrOfMazeRows)
	, MazeTileSize(MazeTileSize)
{
	
	CreateMazeNodes(arrayOfNodes, arrayOfWalls);
	Kruskals(arrayOfWalls);

}

RandomKruskals::~RandomKruskals()
{
}

void RandomKruskals::Kruskals(TArray<FMazeNodeConnection*> copyArrayOfWalls)
{
	//As long as there are walls WHILE
	while (copyArrayOfWalls.Num() > 0)
	{
		//Choose random connection
		int randIdx = FMath::RandRange(0, copyArrayOfWalls.Num() - 1);
		auto randomCon = copyArrayOfWalls[randIdx];

		//Check if nodes don't belong to the same set
		//bool contains = false;
		//for (auto ids: ConnectedNodes[randomCon->FromNodeID])
		//{
		//	if (ConnectedNodes[randomCon->ToNodeID].Contains(ids)) {
		//		contains = true;
		//		break;
		//	}
		//		
		//}

		//if (!contains) {
		//	//Remove the wall
		//	randomCon->IsWall = false;
		//	

		//	//Append the SETS
		//	ConnectedNodes[randomCon->FromNodeID].Append(ConnectedNodes[randomCon->ToNodeID]);
		//	ConnectedNodes[randomCon->ToNodeID].Append(ConnectedNodes[randomCon->FromNodeID]);
		//}
		int fromConnectedNodesRootID = *RootIdMap[randomCon->FromNodeID];
		int toConnectedNodesRootID = *RootIdMap[randomCon->ToNodeID];
		if (fromConnectedNodesRootID != toConnectedNodesRootID)
		{
			if (fromConnectedNodesRootID < toConnectedNodesRootID)
				RootIdMap[randomCon->ToNodeID] = RootIdMap[randomCon->FromNodeID];
			else
				RootIdMap[randomCon->FromNodeID] = RootIdMap[randomCon->ToNodeID];
			//Other with same pointer don't change together at 3 wall
		}

		copyArrayOfWalls.RemoveAt(randIdx);
	}



}

void RandomKruskals::CreateMazeNodes(TArray<FMazeNode*>& arrayOfNodes, TArray<FMazeNodeConnection*>& arrayOfWalls)
{
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
			arrayOfNodes.Add(newMazeNode);
			//Create Node tree
			FNodeTree* nodeTree = new FNodeTree();
			NodeTreeMap.Add(newMazeNode->MazeNodeId, nodeTree);
			RootIdMap.Add(newMazeNode->MazeNodeId, &newMazeNode->MazeNodeId);
			ConnectedNodes.Add({ newMazeNode->MazeNodeId });

			//Add connections (walls) in all valid directions
			for (auto& dir : MazeDirections)
			{
				adjacentCol = (col + dir.X);
				adjacentRow = (row + dir.Y);

				//Check if the directions in the grid are valid
				if (0 <= adjacentCol && adjacentCol < NrOfMazeColumns
					&& 0 <= adjacentRow && adjacentRow < NrOfMazeRows)
				{
					adjacentNodeIdx = adjacentRow * NrOfMazeColumns + adjacentCol;

					if (IsNotUniqueConnection(arrayOfWalls, newMazeNode->MazeNodeId, adjacentNodeIdx))
						continue;

					FMazeNodeConnection* newMazeConnection;
					newMazeConnection = new FMazeNodeConnection();
					newMazeConnection->FromNodeID = newMazeNode->MazeNodeId;
					newMazeConnection->ToNodeID = adjacentNodeIdx;
					newMazeNode->Connections.Add(newMazeConnection);
					arrayOfWalls.Add(newMazeConnection);
				}

			}
		}
	}

}

bool RandomKruskals::IsNotUniqueConnection(TArray<FMazeNodeConnection*>& arrayOfWalls, int nodeIdx, int adjacentIdx) const
{
	for (auto con : arrayOfWalls)
	{
		if (con->FromNodeID == adjacentIdx
			&& con->ToNodeID == nodeIdx) {
			return true;
		}
	}
	return false;
}
