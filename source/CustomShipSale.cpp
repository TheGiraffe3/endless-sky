/* CustomShipSale.cpp
Copyright (c) 2024 by Loymdayddaud

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "CustomShipSale.h"

#include "ConditionsStore.h"
#include "DataNode.h"
#include "GameData.h"
#include "Logger.h"
#include "Planet.h"
#include "Set.h"
#include "Ship.h"

using namespace std;

namespace
{
	const auto show = map<CustomShipSale::SellType, const string> {
		{CustomShipSale::SellType::DEFAULT, ""},
		{CustomShipSale::SellType::IMPORT, "import"}
	};
	// Initially put values at this to know if we found what we're looking for,
	// whilst allowing 0 in the searched list.
	const double DEFAULT = numeric_limits<double>::infinity();
}



void CustomShipSale::Load(const DataNode &node, bool eventChange)
{
	const Set<Sale<Ship>> &items = GameData::Shipyards();
	const Set<Ship> &ships = GameData::Ships();

	bool isAdd = false;
	const Ship *ship = nullptr;
	// Shipyards or ships mode.
	const string mode = node.Token(1);
	name = node.Token(2);
	auto parseValueOrOffset = [&isAdd, &ship, &mode, this](double &amount, const DataNode &line)
	{
		int size = line.Size();
		// Default is 1, because ships can be added only to have a custom sellType.
		if(isAdd)
			amount += size > 2 ? line.Value(2) : 1.;
		else
			amount = size > 1 ? line.Value(1) : 1.;
		// All values are converted into percentages if that is not how they are given (which would be indicated by %)
		// This means that the offset is handled as relative to the modified price instead of the default one.
		// Shipyard changes always are percentages.
		if((mode != "shipyards"
				&& (size == (2 + isAdd)
				|| (size > 2 && line.Token(2 + isAdd) != "%"))))
			toConvert.push_back(make_pair(ship, &amount));
	};

	for(const DataNode &child : node)
	{
		bool remove = child.Token(0) == "remove";
		bool add = child.Token(0) == "add";

		int keyIndex = (add || remove);
		bool hasKey = child.Size() > keyIndex;

		if(!hasKey)
		{
			if(remove)
				Clear();
			else
			{
				child.PrintTrace("Error: skipping entry with empty \"add\":");
				continue;
			}
		}

		const string &key = child.Token(keyIndex);

		bool isValue = key == "value";
		bool isOffset = key == "offset";

		if(remove)
		{
			if(key == "ship" && mode == "ships")
			{
				// If a ship is specified remove only that one. Otherwise clear all of them.
				if(child.Size() >= 3)
				{
					const Ship *ship = ships.Get(child.Token(2));
					relativeShipPrices.erase(ship);
					relativeShipOffsets.erase(ship);
				}
				else
				{
					relativeShipOffsets.clear();
					relativeShipPrices.clear();
				}
			}
			else if(key == "shipyard" && mode == "shipyards")
			{
				// If a shipyard is specified remove only that one. Otherwise clear all of them.
				if(child.Size() >= 3)
				{
					const Sale<Ship> *shipyard = items.Get(child.Token(2));
					relativePrices.erase(shipyard);
					relativeOffsets.erase(shipyard);
				}
				else
				{
					relativeOffsets.clear();
					relativePrices.clear();
				}
			}
			else if(key == "location")
				locationFilter = LocationFilter{};
			else if(key == "conditions")
				conditions = ConditionSet{};
			else
				child.PrintTrace("Skipping unrecognized clearing/deleting:");
		}
		else if(key == "default")
			sellType = SellType::DEFAULT;
		else if(key == "import")
			sellType = SellType::IMPORT;
		else if(key == "location")
		{
			if(!add)
			{
				location = nullptr;
				locationFilter = LocationFilter{};
			}

			// Add either a whole filter or just a planet.
			if(child.Size() >= 2)
			{
				location = GameData::Planets().Get(child.Token(1));
			}
			else if(child.Size() == 1)
				locationFilter.Load(child);
			else
				child.PrintTrace("Warning: use a location filter to choose from multiple planets:");

			if(location && !locationFilter.IsEmpty())
				child.PrintTrace("Warning: location filter ignored due to use of explicit planet:");
		}
		else if(key == "conditions")
		{
			if(!add)
				conditions = ConditionSet{};
			conditions.Load(child);
		}
		// CustomShipSales are separated between ships and shipyards in the data files.
		else if(mode == "ships")
		{
			if(!add)
			{
				if(isValue)
					relativeShipPrices.clear();
				else if(isOffset)
					relativeShipOffsets.clear();
			}

			if(isValue || isOffset)
				for(const DataNode &grandChild : child)
				{
					isAdd = (grandChild.Token(0) == "add");
					ship = ships.Get(grandChild.Token(isAdd));

					if(isValue)
						parseValueOrOffset(relativeShipPrices[ship], grandChild);
					else if(isOffset)
						parseValueOrOffset(relativeShipOffsets[ship], grandChild);
				}
			else
				child.PrintTrace("Skipping unrecognized attribute:");
		}
		else if(mode == "shipyards")
		{
			if(!add)
			{
				if(isValue)
					relativePrices.clear();
				else if(isOffset)
					relativeOffsets.clear();
			}

			if(isValue || isOffset)
				for(const DataNode &grandChild : child)
				{
					isAdd = (grandChild.Token(0) == "add");
					const Sale<Ship> *shipyard = items.Get(grandChild.Token(isAdd));

					if(isValue)
						parseValueOrOffset(relativePrices[shipyard], grandChild);
					else if(isOffset)
						parseValueOrOffset(relativeOffsets[shipyard], grandChild);
				}
			else
				child.PrintTrace("Skipping unrecognized attribute:");
		}
		else
			child.PrintTrace("Skipping unrecognized attribute:");
	}

	if(eventChange)
		FinishLoading();
}



void CustomShipSale::FinishLoading()
{
	for(const auto &it : toConvert)
		if(it.first->Cost() != 0)
			*it.second /= it.first->Cost();
		else
		{
			// Do nothing.
		}
	toConvert.clear();
}



bool CustomShipSale::Add(const CustomShipSale &other, const Planet &planet, const ConditionsStore &store)
{
	cacheValid = false;
	if(!Matches(planet, store))
		Clear();
	if(!other.Matches(planet, store))
		return false;

	// Selltypes are ordered by priority, a higher priority overrides lower ones.
	if(other.sellType > sellType)
	{
		*this = other;
		return true;
	}

	// For prices, take the highest one.
	for(const auto &it : other.relativePrices)
	{
		auto ours = relativePrices.find(it.first);
		if(ours == relativePrices.end())
			relativePrices.emplace(it.first, it.second);
		else if(ours->second < it.second)
			ours->second = it.second;
	}
	// For offsets, add them to each other.
	for(const auto &it : other.relativeOffsets)
	{
		auto ours = relativeOffsets.find(it.first);
		if(ours == relativeOffsets.end())
			relativeOffsets.emplace(it.first, it.second);
		else
			ours->second += it.second;
	}
	// Same thing for Shipyards.
	for(const auto &it : other.relativeShipPrices)
	{
		auto ours = relativeShipPrices.find(it.first);
		if(ours == relativeShipPrices.end())
			relativeShipPrices.emplace(it.first, it.second);
		else if(ours->second < it.second)
			ours->second = it.second;
	}
	for(const auto &it : other.relativeShipOffsets)
	{
		auto ours = relativeShipOffsets.find(it.first);
		if(ours == relativeShipOffsets.end())
			relativeShipOffsets.emplace(it.first, it.second);
		else
			ours->second += it.second;
	}

	return true;
}



double CustomShipSale::GetRelativeCost(const Ship &item) const
{
	// Ship prices have priority over shipyard prices, so consider them first,
	// and only consider the shipyard prices if the ships have no set price.
	auto baseRelative = relativeShipPrices.find(&item);
	double baseRelativePrice = (baseRelative != relativeShipPrices.cend() ? baseRelative->second : DEFAULT);
	if(baseRelativePrice == DEFAULT)
		for(const auto &it : relativePrices)
			if(it.first->Has(&item))
			{
				baseRelativePrice = it.second;
				break;
			}
	auto baseOffset = relativeShipOffsets.find(&item);
	double baseOffsetPrice = (baseOffset != relativeShipOffsets.cend() ? baseOffset->second : DEFAULT);
	for(const auto &it : relativeOffsets)
		if(it.first->Has(&item))
		{
			if(baseOffsetPrice == DEFAULT)
				baseOffsetPrice = 0.;
			baseOffsetPrice += it.second;
		}
	// Apply the relative offset on top of each others, the result being applied on top of the relative price.
	// This means that a ship can be affected by a shipyard offset, a custom ship price, and ship prices.
	if(baseRelativePrice != DEFAULT)
		return baseRelativePrice + (baseOffsetPrice != DEFAULT ? baseRelativePrice * baseOffsetPrice : 0.);
	else if(baseOffsetPrice != DEFAULT)
		return 1. + baseOffsetPrice;
	else
		return 1.;
}



CustomShipSale::SellType CustomShipSale::GetSellType() const
{
	return sellType;
}



const string &CustomShipSale::GetShown(CustomShipSale::SellType sellType)
{
	return show.find(sellType)->second;
}



bool CustomShipSale::Has(const Ship &item) const
{
	if(relativeShipPrices.find(&item) != relativeShipPrices.end())
		return true;
	if(relativeShipOffsets.find(&item) != relativeShipOffsets.end())
		return true;
	for(auto &&sale : relativePrices)
		if(sale.first->Has(&item))
			return true;
	for(auto &&sale : relativeOffsets)
		if(sale.first->Has(&item))
			return true;
	return false;
}



bool CustomShipSale::Matches(const Planet &planet, const ConditionsStore &playerConditions) const
{
	return (location ? location == &planet : locationFilter.Matches(&planet)) &&
		(conditions.IsEmpty() || conditions.Test(playerConditions));
}



bool CustomShipSale::IsEmpty()
{
	return relativePrices.empty() && relativeOffsets.empty() &&
		relativeShipPrices.empty() && relativeShipOffsets.empty();
}



void CustomShipSale::Clear()
{
	*this = CustomShipSale{};
}
