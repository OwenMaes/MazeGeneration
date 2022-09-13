// Fill out your copyright notice in the Description page of Project Settings.


#include "MazeGenerator.h"
#include <Runtime\Core\Public\ProfilingDebugging\ABTesting.h>
#include <Runtime\Engine\Public\DrawDebugHelpers.h>

// Sets default values
AMazeGenerator::AMazeGenerator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AMazeGenerator::GenerateMaze()
{
	double Time = 0;
	FDurationTimer DurationTimer = FDurationTimer(Time);
	DurationTimer.Start();

	CreateMazeGrid();
	CarveMaze();

	DurationTimer.Stop();
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::SanitizeFloat(Time));

	//Debugging
	DrawDebugMazeGrid();
}

// Called when the game starts or when spawned
void AMazeGenerator::BeginPlay()
{
	Super::BeginPlay();

}

void AMazeGenerator::CreateMazeGrid()
{
	

	//Create Maze Nodes and its adjacent connections (walls)
	TArray<FIntVector> MazeDirections = { { 1, 0, 0 }, { 0, 1, 0 }, { -1, 0, 0 }, { 0, -1, 0 } };
	FMazeNode newMazeNode{};
	FMazeNodeConnection newMazeConnection{};
	FVector position{};
	int adjacentNodeIdx{}, adjacentRow{}, adjacentCol{};

	for (size_t row = 0; row < NrOfMazeRows; row++)
	{
		for (size_t col = 0; col < NrOfMazeColumns; col++)
		{
			//Create Maze node
			position = MazeStartPosition;
			//todo match postion with unreal axis (-y)
			position.X += MazeTileSize * col + MazeTileSize / 2;
			position.Y += MazeTileSize * row + MazeTileSize / 2;
			newMazeNode.MazeNodeId = row * NrOfMazeColumns + col;
			newMazeNode.NodePosition = position;

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
					newMazeConnection.FromNodeID = newMazeNode.MazeNodeId;
					newMazeConnection.ToNodeID = adjacentNodeIdx;
					newMazeNode.Connections.Add(newMazeConnection);
				}
			}

			MazeNodeGrid.Add(newMazeNode.MazeNodeId, newMazeNode);
		}
	}
}

void AMazeGenerator::CarveMaze()
{
	//switch maze generating algorithm
	TArray<FMazeNode*> nodePath{};
	int startNodeIDx = 0;
	if (auto startNode = MazeNodeGrid.Find(startNodeIDx))
		DepthFirstSearch(startNode, nodePath);
	
}

void AMazeGenerator::DepthFirstSearch(FMazeNode* node, TArray<FMazeNode*>& nodePath)
{
	//The node is marked visited
	node->IsVisited = true;

	//Shuffle the connection array
	auto nrOfWalls = node->Connections.Num() - 1;
	int swapIdx{};
	for (auto i = 0; i < nrOfWalls; ++i) {
		swapIdx = FMath::RandRange(i, nrOfWalls);
		node->Connections.Swap(i, swapIdx);
	}

	//Choose a random wall that goes to a node not visited yet
	for (auto i = 0; i < nrOfWalls; i++)
	{
		if (auto unvisitedNode = MazeNodeGrid.Find(node->Connections[i].ToNodeID))
		{
			if (unvisitedNode->IsVisited)
				continue;

			//Remove the wall of the connection
			node->Connections[i].IsWall = false;

			//Add current node to node path for backtracking
			nodePath.Add(node);

			//Go to unvisided node
			DepthFirstSearch(unvisitedNode, nodePath);
		}
	}

	//Check if back at start node
	if (nodePath.Num() == 0)
		return;

	//Go back to node with unvisited connections
	auto prevNode = nodePath.Pop();
	DepthFirstSearch(prevNode, nodePath);

}

void AMazeGenerator::DrawDebugMazeGrid()
{
	float thickness = 10.f;
	FVector floorExtent{};
	floorExtent.X = MazeTileSize / 2 - thickness;
	floorExtent.Y = MazeTileSize / 2 - thickness;
	floorExtent.Z = 20.f;
	//Draw debug nodes
	for (auto& node : MazeNodeGrid)
	{
		DrawDebugBox(GetWorld(), node.Value.NodePosition, floorExtent, FColor::Red, true, 15.f, 0, thickness);

		for (auto conn : node.Value.Connections)
		{
			if (conn.IsWall) {
				if (auto adjacentNode = MazeNodeGrid.Find(conn.ToNodeID))
				{
					FVector pos = node.Value.NodePosition + (adjacentNode->NodePosition - node.Value.NodePosition);
					DrawDebugSphere(GetWorld(), pos, 50.f, 32, FColor::Blue, true, 15.f, 0, thickness);
				}
			}
			
		}
	}
}

// Called every frame
void AMazeGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

