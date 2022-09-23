// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MazeGenerator.generated.h"

UENUM(BlueprintType)
enum class EMazeAlgorithm : uint8 {
	RANDOMDEPTHFIRSTSEARCH = 0 UMETA(DisplayName = "Randomized Depth-First Search"),
	RANDOMKRUSKALS = 1 UMETA(DisplayName = "Randomized Kruskal's"),
	RANDOMPRISMS = 2 UMETA(DisplayName = "Randomized Prim's"),
};

USTRUCT()
struct FMazeNodeConnection
{
	GENERATED_BODY()

		FMazeNode* FromNode;
		FMazeNode* ToNode;
	int FromNodeID;
	int ToNodeID;
	bool IsWall;

	FMazeNodeConnection(int fromNodeID, int toNodeID)
		:FromNodeID(fromNodeID)
		,ToNodeID(toNodeID)
		,IsWall(true)
		,FromNode(nullptr)
		,ToNode(nullptr)
	{

	}

	FMazeNodeConnection()
		:FromNodeID(-1)
		, ToNodeID(-1)
		, IsWall(true)
		, FromNode(nullptr)
		, ToNode(nullptr)
	{

	}
};

USTRUCT()
struct FMazeNode
{
	GENERATED_BODY()

	int MazeNodeId;
	FVector NodePosition;
	bool IsVisited;
	TArray<FMazeNodeConnection*> Connections;


	FMazeNode(int nodeID, FVector nodePosition)
		:MazeNodeId(nodeID)
		,NodePosition(nodePosition)
		,IsVisited(false)
		,Connections()
	{
		Connections = {};
	}

	FMazeNode()
		:MazeNodeId(-1)
		,NodePosition()
		,IsVisited(false)
	{
		Connections = {};
	}

};

UCLASS()
class MAZEGENERATION_API AMazeGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMazeGenerator();

	UFUNCTION(BlueprintCallable, Category = "Maze")
		void GenerateMaze();

	/*The start point of the maze.*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Maze settings")
		FVector MazeStartPosition = {};

	/*The amount of columns in the maze (width).*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Maze settings")
		int NrOfMazeColumns = 50;

	/*The amount of rows in the maze (height).*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Maze settings")
		int NrOfMazeRows = 50;

	/*The size of the maze tiles (Floors & Walls).*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Maze settings")
		float MazeTileSize = 600;

	/*The algorithm used to carve the walls (generate maze).*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Maze settings")
		EMazeAlgorithm MazeGenerationAlgorithm = EMazeAlgorithm::RANDOMDEPTHFIRSTSEARCH;

	/*If debug is drawn.*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Maze settings")
		bool DrawDebug = false;

	UPROPERTY(VisibleAnywhere, Category = "Meshes")
		UInstancedStaticMeshComponent* FloorTileISMC;
	UPROPERTY(VisibleAnywhere, Category = "Meshes")
		UInstancedStaticMeshComponent* WallTileISMC;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	TMap<int, FMazeNode> MazeNodeGrid = {};
	TArray<FMazeNode*> MazeNodePath = {};
	TArray<FMazeNode*> MazeNodeArray = {};
	TArray<FMazeNodeConnection*> WallList = {};
	TMap<int, TSet<int>> DividedMazeNodes = {};

	void ResetMazeNodes();
	void ResetVisitedMazeNodes();
	bool FindPathDepthFirstSearch(FMazeNode* startNode, FMazeNode* endNode, TArray<FMazeNode*>& nodePath);
	void SpawnMeshes();
	void SpawnOuterWalls(FTransform& floorTransform, FTransform& wallTransform, FRotator& wallRotation, FVector& wallDirection, FVector& wallPos);
	void SpawnInnerWalls(FTransform& floorTransform, FTransform& wallTransform, FRotator& wallRotation, FVector& wallDirection, FVector& wallPos);
	void SpawnFloors(FTransform& floorTransform);
	void DrawDebugMazeGrid();
	bool CheckIfMazeIsValid();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
