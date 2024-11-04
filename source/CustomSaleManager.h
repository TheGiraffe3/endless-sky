/* CustomSaleManager.h
Copyright (c) 2024 by Hurleveur

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include "CustomOutfitSale.h"
#include "CustomShipSale.h"

#include <map>

class ConditionsStore;
class Outfit;
class Planet;
class Ship;
class System;



class CustomOutfitSaleManager {
public:
	// Refresh the active CustomSales to correspond to the planet and conditions.
	static inline void Refresh(const Planet *planet, const ConditionsStore &conditions);
	// Or to all planets in this system.
	static inline void Refresh(const System *system, const ConditionsStore &conditions);
	// Return if can be bought. And imported item will still be shown but not be buyable.
	static inline bool CanBuy(const Outfit &outfit);

	// Get the full/relative cost of the outfit with the last cached conditions.
	static inline int64_t OutfitCost(const Outfit &outfit);
	static inline double OutfitRelativeCost(const Outfit &outfit);
	// Delete all cached data.
	static inline void Clear();


public:
	static inline std::map<CustomOutfitSale::SellType, CustomOutfitSale> customOutfitSales;
};



class CustomShipSaleManager {
public:
	// Refresh the active CustomSales to correspond to the planet and conditions.
	static inline void Refresh(const Planet *planet, const ConditionsStore &conditions);
	// Or to all planets in this system.
	static inline void Refresh(const System *system, const ConditionsStore &conditions);
	// Return if can be bought. And imported item will still be shown but not be buyable.
	static inline bool CanBuy(const Ship &ship);

	// Get the full/relative cost of the ship with the last cached conditions.
	static inline int64_t ShipCost(const Ship &ship);
	static inline double ShipRelativeCost(const Ship &ship);
	// Delete all cached data.
	static inline void Clear();


public:
	static inline std::map<CustomShipSale::SellType, CustomShipSale> customShipSales;
};
