// BattleTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../BattleCalc/combatsimulator.h"
#include <iostream>

int main()
{
	attacker x;
	defender y;
	x.troops[5] = 20000;
	y.troops[5] = 2000;
	battleResult z;
	CombatSimulator::fight(x, y, &z);
	std::getchar();
}

