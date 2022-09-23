// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomKruskals.h"

RandomKruskals::RandomKruskals()
{
}

RandomKruskals::~RandomKruskals()
{
}

void RandomKruskals::Kruskals()
{
	//TArray<FMazeNodeConnection*> wallList(WallList);
	//for (auto& node : MazeNodeGrid)
	//{
	//	DividedMazeNodes.Add(node.Value.MazeNodeId, { node.Value.MazeNodeId });
	//}

	////As long as there are walls and no unified set of nodes
	//while (wallList.Num() > 0)
	//{
	//	int randIdx = FMath::RandRange(0, wallList.Num() - 1);
	//	auto randomCon = wallList[randIdx];

	//	//Check if the nodes of the connection are from different SETS
	//	auto nodesFromUnified = DividedMazeNodes.Find(randomCon->FromNodeID);
	//	if (!nodesFromUnified->Contains(randomCon->ToNodeID)) {
	//		//Remove the wall
	//		randomCon->IsWall = false;
	//		wallList.RemoveAt(randIdx);

	//		//Append the SETS
	//		auto nodesToUnified = DividedMazeNodes.Find(randomCon->ToNodeID);
	//		nodesFromUnified->Append(nodesToUnified->Array());
	//		nodesToUnified = nodesFromUnified;

	//	}

	//}
}

void RandomKruskals::CreateNodeTrees()
{
}
