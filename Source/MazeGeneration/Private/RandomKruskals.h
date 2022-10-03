// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct FMazeNode;
struct FMazeNodeConnection;

struct FNodeTree
{
	FMazeNode* MazeNode;
	


	FNodeTree()
		:MazeNode(nullptr)
		
	{

	}

};


class RandomKruskals
{
public:
	RandomKruskals(FVector mazeStartPosition, int nrOfMazeColumns, int nrOfMazeRows, float MazeTileSize, TArray<FMazeNode*>& arrayOfNodes, TArray<FMazeNodeConnection*>& arrayOfWalls);
	~RandomKruskals();
	void Kruskals(TArray<FMazeNodeConnection*> copyArrayOfWalls);
	void CreateMazeNodes(TArray<FMazeNode*>& arrayOfNodes, TArray<FMazeNodeConnection*>& arrayOfWalls);
	bool IsNotUniqueConnection(TArray<FMazeNodeConnection*>& arrayOfWalls, int nodeIdx,int adjacentIdx) const;

private:
	FVector MazeStartPosition;
	int NrOfMazeColumns;
	int NrOfMazeRows;
	float MazeTileSize;
	TMap<int, FNodeTree*> NodeTreeMap;
	TMap<int, int*> RootIdMap;
	TArray<TSet<int>> ConnectedNodes;
	

};
