// BattleTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../BattleCalc/combatsimulator.h"
#include <iostream>

int main()
{
	attacker x;
	defender y;
/*	x.troops[5] = 1;
	x.troops[7] = 1;
	x.troops[8] = 124996;
	x.troops[9] = 1;
	x.troops[11] = 1;
	y.troops[1] = 100000;
	y.troops[5] = 1000000;*/
	x.troops[8] = 1000000;
	x.troops[11] = 1;
	y.troops[5] = 100000;
	battleResult z;
	CombatSimulator::fight(x, y, &z);
	std::getchar();
}

