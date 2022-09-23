// Fill out your copyright notice in the Description page of Project Settings.


#include "MazeGenerator.h"
#include "RandomDepthFirstSearch.h"
#include <Runtime\Core\Public\ProfilingDebugging\ABTesting.h>
#include <Runtime\Engine\Public\DrawDebugHelpers.h>
#include <Runtime\Engine\Classes\Kismet\KismetMathLibrary.h>


// Sets default values
AMazeGenerator::AMazeGenerator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	FloorTileISMC = CreateDefaultSubobject<class UInstancedStaticMeshComponent>(TEXT("Floor InstancedStaticMesh"));
	FloorTileISMC->SetMobility(EComponentMobility::Static);
	FloorTileISMC->SetCollisionProfileName("BlockAll");

	WallTileISMC = CreateDefaultSubobject<class UInstancedStaticMeshComponent>(TEXT("Wall InstancedStaticMesh"));
	WallTileISMC->SetMobility(EComponentMobility::Static);
	WallTileISMC->SetCollisionProfileName("BlockAll");

}

void AMazeGenerator::GenerateMaze()
{
	double Time = 0;
	FDurationTimer DurationTimer = FDurationTimer(Time);
	DurationTimer.Start();

	RandomDepthFirstSearch randomDFS(MazeStartPosition, NrOfMazeColumns, NrOfMazeRows, MazeTileSize, MazeNodeArray, WallList);

	DurationTimer.Stop();
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::SanitizeFloat(Time));

	

	////Debugging
	if (DrawDebug)
		DrawDebugMazeGrid();

	//Spawn meshes
	SpawnMeshes();
}

// Called when the game starts or when spawned
void AMazeGenerator::BeginPlay()
{
	Super::BeginPlay();

}

void AMazeGenerator::ResetMazeNodes()
{
	for (auto& node : MazeNodeGrid)
	{
		node.Value.IsVisited = false;
		for (auto& con : node.Value.Connections)
		{
			con->IsWall = true;
		}
	}
}

void AMazeGenerator::ResetVisitedMazeNodes()
{
	//set all nodes to unvisited //Create map of visited nodes.
	for (auto& node : MazeNodeGrid)
	{
		node.Value.IsVisited = false;
	}
}


bool AMazeGenerator::FindPathDepthFirstSearch(FMazeNode* currentNode, FMazeNode* endNode, TArray<FMazeNode*>& nodePath)
{
	//The node is marked visited
	currentNode->IsVisited = true;

	//Exit if node is end goal
	if (currentNode->MazeNodeId == endNode->MazeNodeId) {
		return true;
	}


	//Choose a unvisited node that is closest to the end goal
	float closestUnvisitedNodeDistanceSqrd = FLT_MAX;
	float currentDistanceSqrd = {};
	int nodeIDx = -1;
	for (auto& con : currentNode->Connections)
	{
		if (!con->IsWall)
		{
			if (auto unvistedNode = MazeNodeGrid.Find(con->ToNodeID)) {
				if (!unvistedNode->IsVisited) {
					currentDistanceSqrd = FVector::DistSquared(unvistedNode->NodePosition, endNode->NodePosition);
					if (currentDistanceSqrd < closestUnvisitedNodeDistanceSqrd)
						nodeIDx = unvistedNode->MazeNodeId;
				}
			}
		}
	}

	//Go to next node
	if (nodeIDx != -1)
	{
		if (auto unvistedNode = MazeNodeGrid.Find(nodeIDx)) {
			//Add current node to node path for backtracking
			nodePath.Add(currentNode);

			//Go to unvisided node
			FindPathDepthFirstSearch(unvistedNode, endNode, nodePath);
		}

	}
	else
	{
		if (nodePath.Num() == 0)
			return false;

		//Go back if no unvisited nodes are adjacent
		auto prevNode = nodePath.Pop();
		FindPathDepthFirstSearch(prevNode, endNode, nodePath);
	}

	return nodePath.Num() > 0;
}

void AMazeGenerator::SpawnMeshes()
{
	FTransform floorTransform{};
	FTransform wallTransform{};
	FRotator wallRotation{};
	FVector wallDirection{};
	FVector wallPos{ 0,0,0 };

	SpawnOuterWalls(floorTransform, wallTransform, wallRotation, wallDirection, wallPos);

	SpawnFloors(floorTransform);

	SpawnInnerWalls(floorTransform, wallTransform, wallRotation, wallDirection, wallPos);
}

void AMazeGenerator::SpawnOuterWalls(FTransform& floorTransform, FTransform& wallTransform, FRotator& wallRotation, FVector& wallDirection, FVector& wallPos)
{
	//outer walls along X-axis
	wallRotation = UKismetMathLibrary::FindLookAtRotation({ 0,1,0 }, { 0,0,0 });
	wallTransform.SetRotation(wallRotation.Quaternion());
	for (size_t col = 0; col < NrOfMazeColumns; col++)
	{
		wallPos.X = MazeStartPosition.X + col * MazeTileSize - MazeTileSize / 2;
		//Walls on the BOTTOM
		wallPos.Y = MazeStartPosition.Y;
		wallTransform.SetLocation(wallPos);
		WallTileISMC->AddInstanceWorldSpace(wallTransform);
		//Walls on the TOP
		wallPos.Y = MazeStartPosition.Y - NrOfMazeRows * MazeTileSize;
		wallTransform.SetLocation(wallPos);
		WallTileISMC->AddInstanceWorldSpace(wallTransform);
	}
	//outer walls along Y-axis
	wallRotation = UKismetMathLibrary::FindLookAtRotation({ 1,0,0 }, { 0,0,0 });
	wallTransform.SetRotation(wallRotation.Quaternion());
	for (size_t row = 0; row < NrOfMazeRows; row++)
	{
		wallPos.Y = MazeStartPosition.Y - row * MazeTileSize - MazeTileSize / 2;
		//Walls on the LEFT
		wallPos.X = MazeStartPosition.X - MazeTileSize;
		wallTransform.SetLocation(wallPos);
		WallTileISMC->AddInstanceWorldSpace(wallTransform);
		//Walls on the Right
		wallPos.X = MazeStartPosition.X + MazeTileSize * NrOfMazeColumns - MazeTileSize;
		wallTransform.SetLocation(wallPos);
		WallTileISMC->AddInstanceWorldSpace(wallTransform);
	}
}

void AMazeGenerator::SpawnInnerWalls(FTransform& floorTransform, FTransform& wallTransform, FRotator& wallRotation, FVector& wallDirection, FVector& wallPos)
{
	for (auto conn : WallList)
	{
		if (conn->IsWall) {
			auto fromNode = conn->FromNode;
			auto toNode = conn->ToNode;
			if (fromNode && toNode)
			{
				//Spawn wall
				wallDirection = (toNode->NodePosition - fromNode->NodePosition);
				wallDirection.Normalize();
				wallRotation = UKismetMathLibrary::FindLookAtRotation(wallDirection, { 0,0,0 });
				wallTransform.SetRotation(wallRotation.Quaternion());
				wallTransform.SetLocation(fromNode->NodePosition + wallDirection * (MazeTileSize / 2));
				WallTileISMC->AddInstanceWorldSpace(wallTransform);
			}
		}


	}
}

void AMazeGenerator::SpawnFloors(FTransform& floorTransform)
{
	for (auto node : MazeNodeArray)
	{
		floorTransform.SetLocation(node->NodePosition);
		FloorTileISMC->AddInstanceWorldSpace(floorTransform);
	}
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
		DrawDebugString(GetWorld(), node.Value.NodePosition, FString::FromInt(node.Value.MazeNodeId));
		/*for (auto conn : node.Value.Connections)
		{
			if (conn.IsWall) {
				if (auto adjacentNode = MazeNodeGrid.Find(conn.ToNodeID))
				{
					FVector pos = node.Value.NodePosition + (adjacentNode->NodePosition - node.Value.NodePosition);
					DrawDebugSphere(GetWorld(), pos, 50.f, 32, FColor::Blue, true, 15.f, 0, thickness);
				}
			}

		}*/
	}

	//Draw maze path
	/*for (auto node : MazeNodePath)
	{
		DrawDebugSphere(GetWorld(), node->NodePosition, 100.f, 10, FColor::Red, true, 5.f, 0, 15.f);
	}*/
}

bool AMazeGenerator::CheckIfMazeIsValid()
{
	//Check if maze is valid
	ResetVisitedMazeNodes();
	auto startNode = MazeNodeGrid.Find(0);
	auto endNode = MazeNodeGrid.Find(MazeNodeGrid.Num() - 1);
	bool isValid = FindPathDepthFirstSearch(startNode, endNode, MazeNodePath);

	if (DrawDebug)
	{
		FString text{};
		FColor color{};
		if (isValid) {
			MazeNodePath.Add(endNode);
			text = "Finding path... End node is reachable, path nodes: " + FString::FromInt(MazeNodePath.Num());
			color = FColor::Green;
			isValid = true;
		}
		else {
			text = "Finding path... End node is NOT reachable, path nodes: 0";
			color = FColor::Red;
			isValid = false;
		}
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, color, text);
	}

	return isValid;
}

// Called every frame
void AMazeGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

