// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
struct FMazeNode;
struct FMazeNodeConnection;
class MAZEGENERATION_API RandomDepthFirstSearch: public FNonAbandonableTask
{
public:
	RandomDepthFirstSearch(FVector mazeStartPosition, int nrOfMazeColumns, int nrOfMazeRows, float MazeTileSize, TArray<FMazeNode*>& arrayOfNodes, TArray<FMazeNodeConnection*>& arrayOfWalls);
	~RandomDepthFirstSearch();

	void DoWork();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(RandomDepthFirstSearch, STATGROUP_ThreadPoolAsyncTasks)
	}

private:
	void CreateMazeGrid(TArray<FMazeNodeConnection*>& arrayOfWalls);
	void CarvePath();
	void RandomDFS(FMazeNode* node, TArray<FMazeNode*>& nodePath);

	FVector MazeStartPosition;
	int NrOfMazeColumns;
	int NrOfMazeRows;
	float MazeTileSize;
	TMap<int, FMazeNode*> MazeNodeGrid;
	TArray<FMazeNode*> MazeNodePath;

	TArray<FMazeNodeConnection*>& ArrayOfWalls;
	TArray<FMazeNode*>& ArrayOfNodes;

};
