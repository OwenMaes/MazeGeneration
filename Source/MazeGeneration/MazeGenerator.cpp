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
	CreateMazeGrid();
	DrawDebugMazeGrid();
}

// Called when the game starts or when spawned
void AMazeGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMazeGenerator::CreateMazeGrid()
{
	double Time = 0;
	FDurationTimer DurationTimer = FDurationTimer(Time);
	DurationTimer.Start();

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
					newMazeNode.Walls.Add(newMazeConnection);
				}
			}

			MazeNodeGrid.Add(newMazeNode.MazeNodeId, newMazeNode);
		}
	}


	DurationTimer.Stop();
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::SanitizeFloat(Time));
}

void AMazeGenerator::DrawDebugMazeGrid()
{
	float thickness = 10.f;
	FVector floorExtent{};
	floorExtent.X = MazeTileSize / 2 - thickness;
	floorExtent.Y = MazeTileSize / 2 - thickness;
	floorExtent.Z = 20.f;
	//Draw debug nodes
	for (auto& node: MazeNodeGrid)
	{
		DrawDebugBox(GetWorld(), node.Value.NodePosition, floorExtent, FColor::Green, true, 15.f, 0, thickness);
	}
}

// Called every frame
void AMazeGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

