// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MazeGenerator.generated.h"

USTRUCT()
struct FMazeNodeConnection
{
	GENERATED_BODY()

	int FromNodeID;
	int ToNodeID;
	bool IsWall;

	FMazeNodeConnection(int fromNodeID, int toNodeID)
		:FromNodeID(fromNodeID)
		,ToNodeID(toNodeID)
		,IsWall(true)
	{

	}

	FMazeNodeConnection()
		:FromNodeID(-1)
		, ToNodeID(-1)
		, IsWall(true)
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
	TArray<FMazeNodeConnection> Connections;
	TArray<FMazeConnection*> MazeConnections;


	FMazeNode(int nodeID, FVector nodePosition)
		:MazeNodeId(nodeID)
		,NodePosition(nodePosition)
		,IsVisited(false)
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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	TMap<int, FMazeNode> MazeNodeGrid = {};

	void CreateMazeGrid();
	void CarveMaze();
	void DepthFirstSearch(FMazeNode* node, TArray<FMazeNode*>& nodePath);
	void DrawDebugMazeGrid();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
