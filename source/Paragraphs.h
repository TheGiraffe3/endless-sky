/* Paragraphs.h
Copyright (c) 2024 by an anonymous author

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

#include "ConditionSet.h"
#include "ConditionsStore.h"

#include <string>
#include <utility>
#include <vector>

class DataNode;



// Stores a list of description paragraphs, and a condition under which each should be shown.
// See the planet and spaceport description code for examples.
class Paragraphs {
public:
	using const_iterator = std::vector<std::pair<ConditionSet, std::string>>::const_iterator;


public:
	// Load one line of text and possible conditions from the given node.
	void Load(const DataNode &node);

	// Discard all description lines.
	void Clear();

	bool operator==(const Paragraphs &other) const { return text == other.text; }
	bool operator!=(const Paragraphs &other) const { return !(*this == other); }

	// Is this object totally void of all information?
	bool IsEmpty() const;

	// Are there any lines which match these vars?
	bool IsEmptyFor(const ConditionsStore &vars) const;

	// Concatenate all lines which match these vars.
	std::string ToString(const ConditionsStore &vars = {}) const;

	// Iterate over all text. Needed to support PrintData.
	// These must use standard naming conventions (begin, end) for compatibility with range-based for loops.
	const_iterator begin() const;
	const_iterator end() const;


private:
	std::vector<std::pair<ConditionSet, std::string>> text;
};
