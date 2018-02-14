// BattleCalc.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "combatsimulator.h"
#include <array>
#include <iostream>

std::array<int32_t,5> CombatSimulator::meleeTroopTypes = { 0,1,2,3,4 };
std::array<int32_t,3> CombatSimulator::rangedTroopTypes = { 5,9,11 };
std::array<int32_t,3> CombatSimulator::mechTroopTypes = { 9,10,11 };
std::array<int32_t,2> CombatSimulator::mountedTroopTypes = { 7,8 };
std::array<int32_t,6> CombatSimulator::groundTroopTypes = { 0,1,2,3,4,5 };
troopStat CombatSimulator::baseStats[12] = {
	troopStat{ 100,5,50,180,10 }, // worker
	troopStat{ 200,50,50,200,20 }, // warrior
	troopStat{ 100,20,50,3000,20 }, // scout
	troopStat{ 300,150,150,300,50 }, // pike
	troopStat{ 350,100,250,275,30 }, // swords
	troopStat{ 250,120,50,250,1200 }, // archer
	troopStat{ }, //transporter :: TODO
	troopStat{ 500,250,180,1000,100 }, // cavalry
	troopStat{ 1000,350,350,750,80 }, // cataphract
	troopStat{ 320,450,160,100,1400 }, // ballista
	troopStat{ 5000,250,160,120,600 }, // ram
	troopStat{ 480,600,200,80,1500 } // catapult
};
troopStat CombatSimulator::baseFortificationStats[5] = {
	troopStat{}, // trap :: TODO
	troopStat{}, // abatis :: TODO
	troopStat{ 2000,300,360,0,1300 }, // archer tower
	troopStat{}, // rolling logs :: TODO
	troopStat{} // trebuchet :: TODO
};
double CombatSimulator::damageModifiers[12][12] = {
	{}, // modifiers for worker attacking other troops
	{}, // for warrior
	{}, // for scout
	{ 0,0,0,0,0,0,0,1.6,1.6 }, // for pike
	{ 0,0,0,1.1 }, // for swords
	{}, // for archer
	{}, // for transporter :: TODO
	{ 0,0,0,0,0,1.2 }, // for cavs
	{ 0,0,0,0,0,1.2 }, // for cataphracts
	{}, // for ballista
	{}, // for ram
	{} // for catapult
};
void CombatSimulator::modifyStats(troopStat* base, researchStats res, heroStat hero, double atk_modifier, double def_modifier, double life_modifier) {
	for (int i = 0; i < 12; i++) {
		base[i].defense = max(1 - (res.iron_working * 5 + 100)*(100 + hero.intel)*base[i].defense*def_modifier / 10000000, 0.5);
	}
	for (int i : groundTroopTypes) {
		base[i].life = base[i].life*(res.medicine * 5 + 100)*life_modifier / 100;
		base[i].attack = base[i].attack*atk_modifier;
		base[i].speed = base[i].speed*(res.compass * 10 + 100) / 100;
	}
	for (int i : mechTroopTypes) {
		base[i].life = base[i].life*life_modifier;
		base[i].attack = base[i].attack*atk_modifier;
		base[i].speed = base[i].speed*(res.horseback_riding * 5 + 100) / 100;
	}
	for (int i : rangedTroopTypes) {
		base[i].range = base[i].range*(res.archery * 5 + 100) / 100;
	}
	for (int i : mountedTroopTypes) {
		base[i].life = base[i].life*life_modifier;
		base[i].attack = base[i].attack*atk_modifier;
		base[i].speed = base[i].speed*(res.horseback_riding * 5 + 100) / 100;
	}
}
int8_t CombatSimulator::compareSpeed(combatTroops& x, combatTroops& y) {
	if (x.stat->speed == y.stat->speed) {
		if (x.isAttacker) return false;
		else return true;
	}
	return (x.stat->speed > y.stat->speed);
}
// id for the fortifications are reduced by 14
void CombatSimulator::fight(attacker atk, defender def,battleResult* br) {
	troopStat attackerTroopStats[12];
	std::copy(baseStats, baseStats + 12, attackerTroopStats);
	troopStat defenderTroopStats[12];
	std::copy(baseStats, baseStats + 12, defenderTroopStats);
	modifyStats(attackerTroopStats, atk.research, atk.hero, atk.attack_modifier, atk.defence_modifier, atk.life_modifier);
	modifyStats(defenderTroopStats, def.research, def.hero, def.attack_modifier, def.defence_modifier, def.life_modifier);
	
	// modifying archer tower stats
	// TODO: other wall defenses
	troopStat defenderFortificationStats[5];
	std::copy(baseFortificationStats,baseFortificationStats+5,defenderFortificationStats);
	defenderFortificationStats[2].life = (def.research.medicine * 5 + 100)*defenderFortificationStats[2].life / 100;
	defenderFortificationStats[2].attack = defenderFortificationStats[2].attack*def.attack_modifier;
	defenderFortificationStats[2].defense = max(1 - (def.research.iron_working * 5 + 100)*(100 + def.hero.intel)*defenderFortificationStats[2].defense*def.defence_modifier / 10000000, 0.5);
	defenderFortificationStats[2].range = defenderFortificationStats[2].range*(def.research.archery * 5 + def.wallLevel*4.5 + 100) / 100;
	combatTroops combatTroopsArray[24];
	// Calculating field size
	int field_size = 0;
	for (int i = 0; i < 12; i++) {
		if (atk.troops[i] > 0 || def.troops[i] > 0) field_size = max(field_size, baseStats[i].range + baseStats[i].speed);
	}

	// Creating an array with both attacking troops and defending troops for combat
	for (int i = 0; i < 12; i++) {
		combatTroopsArray[i].count = atk.troops[i];
		combatTroopsArray[i].location = 0;
		combatTroopsArray[i].typeId = i;
		combatTroopsArray[i].stat = &attackerTroopStats[i];
		combatTroopsArray[i].isAttacker = true;
	}
	for (int i = 0; i < 12; i++) {
		combatTroopsArray[i + 12].count = def.troops[i];
		combatTroopsArray[i + 12].location = field_size;
		combatTroopsArray[i + 12].typeId = i;
		combatTroopsArray[i + 12].stat = &defenderTroopStats[i];
	}

	// Sorting the troops array according to speed
	std::sort(combatTroopsArray, combatTroopsArray + 24, compareSpeed);

	int8_t atkerType;
	double atkValue;
	combatTroops* defenderTroop;
	double damageModifier;
	int8_t inRange;
	int64_t damage;
	int64_t damage1;
	int8_t attackerAlive;
	int8_t defenderAlive;
	int32_t maxRange;
	int round;
	for (round = 0; round < 100; ++round) {

		// first move the troops according to the movement order
		for (int i = 0; i < 24; i++) {
			combatTroops& pq = combatTroopsArray[i]; 
			if (pq.count > 0) {
				int32_t nearestPosition = pq.isAttacker?field_size:0;
				inRange=false;
				atkerType = pq.isAttacker;
				maxRange = pq.location + pq.stat->range*(pq.isAttacker?1:-1);
				// checking if some troop is already in its range
				for (int j = 0; j < 24; j++) {
					combatTroops& xp = combatTroopsArray[j];
					if (xp.count > 0 && xp.isAttacker != atkerType) {
						if (xp.location==maxRange||xp.location==pq.location||((xp.location < maxRange) ^ (xp.location < pq.location))) {
							inRange=true;
						}
						if (pq.isAttacker) {
							if (xp.location>=pq.location) nearestPosition=min(nearestPosition,xp.location);
						}
						else {
							if (xp.location<=pq.location) nearestPosition=max(nearestPosition,xp.location);
						}
					}
				}

				// if no troop is in range, move
				if (!inRange) {
					if (pq.isAttacker) pq.location = min(max(nearestPosition-pq.stat->range,pq.location),pq.location+pq.stat->speed);
					else pq.location = max(min(nearestPosition+pq.stat->range,pq.location),pq.location-pq.stat->speed);
#if _DEBUG
					std::cout << (pq.isAttacker ? "Attacker" : "Defender") << " troops of type " << pq.typeId << " moves to " << pq.location << "\n";
#endif
				}
			}
		}

		// now all troops attack each other
		for (int i=0; i<24; i++) {
			combatTroops& pq=combatTroopsArray[i];
			atkerType = pq.isAttacker;
			if (pq.count==0) continue;

			// choose which troop to attack
			combatTroops* defenderTroop=NULL;
			atkValue=0;
			maxRange=pq.location + pq.stat->range*(pq.isAttacker?1:-1);
			for (int j=0;j<24;j++) {
				combatTroops& lp=combatTroopsArray[j];
				if (lp.count>0 && lp.isAttacker!=pq.isAttacker) {
					if ((lp.location==maxRange)||(lp.location==pq.location)||((lp.location < maxRange) ^ (lp.location < pq.location))) {
						if ((lp.stat->attack*lp.count)>atkValue) {
							atkValue=lp.stat->attack*lp.count;
							defenderTroop=&lp;
						}
					}
				}
			}

			// attack if there is a troop in range
			if (defenderTroop!=NULL) {
				damageModifier = 0;

				// checking if attacker is a ranged troop
				if (pq.typeId == 5 || pq.typeId == 9 || pq.typeId == 11) {
					inRange = false;

					// checking if the attacker is in range of a non-ranged troop
					for (combatTroops& xp : combatTroopsArray) {
						if (xp.count > 0 && xp.isAttacker != atkerType && xp.typeId != 5 && xp.typeId != 9 && xp.typeId != 11) {
							// checking if the attacking troop is in range
							if (pq.location==xp.location||pq.location==(xp.location + xp.stat->range*(xp.isAttacker?1:-1))||((pq.location < (xp.location + xp.stat->range*(xp.isAttacker?1:-1))) ^ (pq.location < xp.location))) {
								inRange = true;
								break;
							}
						}
					}
					if (inRange) damageModifier = 0.25;
					// if the opposing troop is a melee troop, modifier is 0.75
					else if (defenderTroop->typeId < 8 && defenderTroop->typeId != 5) damageModifier = 0.75;
					// otherwise damage modifier is 0.5
					else damageModifier = 0.5;
					// when attacking ranged troops or ram, the formula is a bit different
					if (defenderTroop->typeId >= 8 || defenderTroop->typeId == 5) {
						defenderTroop->damage += ceil((pq.stat->attack*defenderTroop->stat->defense*pq.count*damageModifier) / defenderTroop->stat->life);
						continue;
					}
				}

				if (damageModifiers[pq.typeId][defenderTroop->typeId] != 0) damageModifier = damageModifiers[pq.typeId][defenderTroop->typeId];
				if (damageModifier == 0) damageModifier = 1;
				// calculating damage by the attacker
//				std::cout << (pq.isAttacker ? "Attacker" : "Defender") << " troops of type " << pq.typeId << " kills " << ((pq.stat->attack*defenderTroop->stat->defense*pq.count*damageModifier) / defenderTroop->stat->life) << " troops of type " << defenderTroop->typeId << "\n";
				defenderTroop->damage += (pq.stat->attack*defenderTroop->stat->defense*pq.count*damageModifier) / defenderTroop->stat->life;
			}
		}

		// calculating total damage for all the troops and reducing troop count
		attackerAlive = false;
		defenderAlive = false;
		for (combatTroops& xp:combatTroopsArray) {
			if (xp.count>0) {
				if (xp.damage>0) {
					xp.count=max(0,xp.count-floor(xp.damage));
#if _DEBUG
					std::cout << "Killed " << floor(xp.damage) << (xp.isAttacker?" attacker":" defender") << " troops of type " << xp.typeId << "\n";
#endif
					xp.damage=0;
				}
				if (xp.count>0) {
					if (xp.isAttacker) attackerAlive=true;
					else defenderAlive=true;
				}
			}
		}
#if _DEBUG
		std::cout << "==== ROUND " << round << " END ==== \n";
#endif
		if (!attackerAlive || !defenderAlive) break;
	}
	std::copy(atk.troops, atk.troops + 12, br->attackerTroops);
	std::copy(def.troops, def.troops + 12, br->defenderTroops);
	for (combatTroops& uh : combatTroopsArray) {
		if (uh.isAttacker) br->attackerTroops[uh.typeId] = uh.count;
		else br->defenderTroops[uh.typeId] = uh.count;
	}
	br->result = attackerAlive | (defenderAlive << 1);
	br->totalRounds = min(round + 1,100);
#if _DEBUG
	std::cout << "==== Remaining Troops ====\n";
	for (combatTroops& hu : combatTroopsArray) {
		if (hu.count > 0) {
			std::cout << (hu.isAttacker ? "Attacker" : "Defender") << " troop of type " << hu.typeId << " count " << hu.count << "\n";
		}
	}
#endif
}