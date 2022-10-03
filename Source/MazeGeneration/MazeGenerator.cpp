// Fill out your copyright notice in the Description page of Project Settings.


#include "MazeGenerator.h"
#include "RandomDepthFirstSearch.h"
#include "Private\RandomKruskals.h"
#include <Runtime\Core\Public\ProfilingDebugging\ABTesting.h>
#include <Runtime\Engine\Public\DrawDebugHelpers.h>
#include <Runtime\Engine\Classes\Kismet\KismetMathLibrary.h>


// Sets default values
AMazeGenerator::AMazeGenerator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	FloorTileISMC = CreateDefaultSubobject<class UInstancedStaticMeshComponent>(TEXT("Floor InstancedStaticMesh"));
	FloorTileISMC->SetMobility(EComponentMobility::Static);
	FloorTileISMC->SetCollisionProfileName("BlockAll");

	InnerWallTileISMC = CreateDefaultSubobject<class UInstancedStaticMeshComponent>(TEXT("Inner Wall InstancedStaticMesh"));
	InnerWallTileISMC->SetMobility(EComponentMobility::Static);
	InnerWallTileISMC->SetCollisionProfileName("BlockAll");

	OuterWallTileISMC = CreateDefaultSubobject<class UInstancedStaticMeshComponent>(TEXT("Outer Wall InstancedStaticMesh"));
	OuterWallTileISMC->SetMobility(EComponentMobility::Static);
	OuterWallTileISMC->SetCollisionProfileName("BlockAll");

}

void AMazeGenerator::GenerateMaze()
{
	double Time = 0;
	FDurationTimer DurationTimer = FDurationTimer(Time);
	DurationTimer.Start();

	if (MazeGenerationAlgorithm == EMazeAlgorithm::RANDOMDEPTHFIRSTSEARCH)
		GenerateDFSMaze();
	else if (MazeGenerationAlgorithm == EMazeAlgorithm::RANDOMKRUSKALS)
		RandomKruskals randomKruskals(MazeStartPosition, NrOfMazeColumns, NrOfMazeRows, MazeTileSize, MazeNodeArray, WallList);

	DurationTimer.Stop();
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::SanitizeFloat(Time));

	////Debugging
	if (DrawDebug)
		DrawDebugMazeGrid();

	//Spawn meshes
	SpawnMeshes();

	//Create next maze
	if (MazeGenerationAlgorithm == EMazeAlgorithm::RANDOMDEPTHFIRSTSEARCH)
		GenerateDFSMazeAsync();
	
}

// Called when the game starts or when spawned
void AMazeGenerator::BeginPlay()
{
	Super::BeginPlay();

}

void AMazeGenerator::SpawnMeshes(bool isSpawningFloors, bool isSpawningOuterWalls, bool IsSpawningInnerWalls)
{
	FTransform floorTransform{};
	FTransform wallTransform{};
	FRotator wallRotation{};
	FVector wallDirection{};
	FVector wallPos{ 0,0,0 };

	if(isSpawningOuterWalls)
		SpawnOuterWalls(floorTransform, wallTransform, wallRotation, wallDirection, wallPos);

	if(isSpawningFloors)
		SpawnFloors(floorTransform);

	if(IsSpawningInnerWalls)
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
		OuterWallTileISMC->AddInstanceWorldSpace(wallTransform);
		//Walls on the TOP
		wallPos.Y = MazeStartPosition.Y - NrOfMazeRows * MazeTileSize;
		wallTransform.SetLocation(wallPos);
		OuterWallTileISMC->AddInstanceWorldSpace(wallTransform);
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
		OuterWallTileISMC->AddInstanceWorldSpace(wallTransform);
		//Walls on the Right
		wallPos.X = MazeStartPosition.X + MazeTileSize * NrOfMazeColumns - MazeTileSize;
		wallTransform.SetLocation(wallPos);
		OuterWallTileISMC->AddInstanceWorldSpace(wallTransform);
	}
}

void AMazeGenerator::SpawnInnerWalls(FTransform& floorTransform, FTransform& wallTransform, FRotator& wallRotation, FVector& wallDirection, FVector& wallPos)
{
	for (auto conn : WallList)
	{
		if (conn->IsWall) {
			auto fromNode = MazeNodeArray[conn->FromNodeID];
			auto toNode = MazeNodeArray[conn->ToNodeID];
			if (fromNode && toNode)
			{
				//Spawn wall
				wallDirection = (toNode->NodePosition - fromNode->NodePosition);
				wallDirection.Normalize();
				wallRotation = UKismetMathLibrary::FindLookAtRotation(wallDirection, { 0,0,0 });
				wallTransform.SetRotation(wallRotation.Quaternion());
				wallTransform.SetLocation(fromNode->NodePosition + wallDirection * (MazeTileSize / 2));
				InnerWallTileISMC->AddInstanceWorldSpace(wallTransform);
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
	for (auto node : MazeNodeArray)
	{
		DrawDebugString(GetWorld(), node->NodePosition, FString::FromInt(node->MazeNodeId));
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

void AMazeGenerator::GenerateDFSMazeAsync()
{
	(new FAutoDeleteAsyncTask<RandomDepthFirstSearch>(MazeStartPosition, NrOfMazeColumns, NrOfMazeRows,
		MazeTileSize, MazeNodeArray, WallList))->StartBackgroundTask();
}

void AMazeGenerator::GenerateDFSMaze()
{
	RandomDepthFirstSearch randomDFS(MazeStartPosition, NrOfMazeColumns, NrOfMazeRows,
		MazeTileSize, MazeNodeArray, WallList);
	randomDFS.DoWork();
}

void AMazeGenerator::GenerateKruskalsMazeAsync()
{
}

void AMazeGenerator::UpdateChangeMaze(float delta)
{
	InnerWallTileISMC->ClearInstances();
	SpawnMeshes(false, false, true);

	ElapsedTimeUntilMazeChange += delta;
	if (ElapsedTimeUntilMazeChange >= MazeChangeTimer) {
		ElapsedTimeUntilMazeChange = 0;

		switch (MazeGenerationAlgorithm)
		{
		case EMazeAlgorithm::RANDOMDEPTHFIRSTSEARCH:
			GenerateDFSMazeAsync();
			break;
		case EMazeAlgorithm::RANDOMKRUSKALS:
			break;
		case EMazeAlgorithm::RANDOMPRISMS:
			break;
		default:
			break;
		}
	}
}

// Called every frame
void AMazeGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateChangeMaze(DeltaTime);
}

