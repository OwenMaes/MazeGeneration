// Fill out your copyright notice in the Description page of Project Settings.


#include "MazeGenerator.h"
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

	CreateMazeGrid();
	CarveMaze();

	DurationTimer.Stop();
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::SanitizeFloat(Time));

	while (!CheckIfMazeIsValid())
	{
		ResetMazeNodes();
		CarveMaze();
	}

	//Debugging
	if(DrawDebug)
		DrawDebugMazeGrid();

	//Spawn meshes
	SpawnMeshes();
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
			position.X += MazeTileSize * col - MazeTileSize / 2;
			position.Y -= MazeTileSize * row + MazeTileSize / 2;
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
			newMazeNode.Connections.Empty();
		}
	}
}

void AMazeGenerator::CarveMaze()
{
	//switch maze generating algorithm
	int startNodeIDx = 0;
	TArray<FMazeNode*> backtrackPath{};
	if (auto startNode = MazeNodeGrid.Find(startNodeIDx))
		RandomDepthFirstSearch(startNode, backtrackPath);
}

void AMazeGenerator::ResetMazeNodes()
{
	for (auto& node : MazeNodeGrid)
	{
		node.Value.IsVisited = false;
		for (auto& con: node.Value.Connections)
		{
			con.IsWall = true;
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

void AMazeGenerator::RandomDepthFirstSearch(FMazeNode* node, TArray<FMazeNode*>& nodePath)
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
			if (auto adjacentNode = MazeNodeGrid.Find(node->Connections[i].ToNodeID)) {
				for (auto& conn: adjacentNode->Connections)
				{
					if (conn.ToNodeID == node->Connections[i].FromNodeID)
					{
						conn.IsWall = false;
						break;
					}
				}
			}

			//Add current node to node path for backtracking
			nodePath.Add(node);
	
			//Go to unvisided node
			RandomDepthFirstSearch(unvisitedNode, nodePath);
		}
	}

	//Check if back at start node
	if (nodePath.Num() == 0)
		return;

	//Go back to node with unvisited connections
	auto prevNode = nodePath.Pop();
	RandomDepthFirstSearch(prevNode, nodePath);

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
	for (auto& con: currentNode->Connections)
	{
		if (!con.IsWall)
		{
			if (auto unvistedNode = MazeNodeGrid.Find(con.ToNodeID)) {
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

	for (auto& node : MazeNodeGrid)
	{
		//Spawn floor
		floorTransform.SetLocation(node.Value.NodePosition);
		FloorTileISMC->AddInstanceWorldSpace(floorTransform);

		for (auto conn : node.Value.Connections)
		{
			if (conn.IsWall) {
				if (auto adjacentNode = MazeNodeGrid.Find(conn.ToNodeID))
				{
					//Spawn wall
					wallDirection = (adjacentNode->NodePosition - node.Value.NodePosition);
					wallDirection.Normalize();
					wallRotation = UKismetMathLibrary::FindLookAtRotation(wallDirection, { 0,0,0 });
					wallTransform.SetRotation(wallRotation.Quaternion());
					wallTransform.SetLocation(node.Value.NodePosition + wallDirection * (MazeTileSize / 2));
					WallTileISMC->AddInstanceWorldSpace(wallTransform);
				}
			}

		}
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
	for (auto node: MazeNodePath)
	{
		DrawDebugSphere(GetWorld(), node->NodePosition, 100.f, 10, FColor::Red, true, 5.f, 0, 15.f);
	}
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

