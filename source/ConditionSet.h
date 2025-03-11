/* ConditionSet.h
Copyright (c) 2014-2024 by Michael Zahniser and others

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

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

class ConditionsStore;
class DataNode;
class DataWriter;



// A condition set is a collection of operations on the player's set of named "conditions"; "test" operations that just
// check the values of those conditions and "evaluation" operations that can calculate an int64_t value based on the
// conditions.
class ConditionSet {
public:
	enum class ExpressionOp {
		INVALID, ///< Expression is invalid.

		// Direct access operators
		VAR, ///< Direct access to condition variable, no other operations.
		LIT, ///< Direct access to literal, no other operations).

		// Arithmetic operators
		ADD, ///< Adds ( + ) the values from all sub-expressions.
		SUB, ///< Subtracts ( - ) all later sub-expressions from the first one.
		MUL, ///< Multiplies ( * ) all sub-expressions with each-other.
		DIV, ///< (Integer) Divides ( / ) the first sub-expression by all later ones.
		MOD, ///< Modulo ( % ) by the second and later sub-expressions on the first one.

		// Boolean equality operators, return 0 or 1
		EQ, ///< Tests for equality ( == ).
		NE, ///< Tests for not equal to ( != ).
		LE, ///< Tests for less than or equal to ( <= ).
		GE, ///< Tests for greater than or equal to ( >= ).
		LT, ///< Tests for less than ( < ).
		GT, ///< Tests for greater than ( > ).

		// Boolean combination operators, return 0 or 1
		AND, ///< Boolean 'and' operator; returns 0 on first 0 subcondition, value of first sub-condition otherwise.
		OR, ///< Boolean 'or' operator; returns value of first non-zero sub-condition, or zero if all are zero.

		// Single boolean operators
		NOT, ///< Single boolean 'not' operator.
		HAS ///< Single boolean 'has' operator.
	};


public:
	ConditionSet() = default;
	ConditionSet(const ConditionSet &) = default;

	ConditionSet &operator=(const ConditionSet &&other) noexcept;
	ConditionSet &operator=(const ConditionSet &other);

	// Construct and Load() at the same time.
	explicit ConditionSet(const DataNode &node);

	// Construct a terminal with a literal value.
	explicit ConditionSet(int64_t newLiteral);

	// Load a set of conditions from the children of this node. Prints a
	// warning if the conditions cannot be parsed from the node.
	void Load(const DataNode &node);
	// Save a set of conditions.
	void Save(DataWriter &out) const;
	void SaveChild(int childNr, DataWriter &out) const;
	void SaveSubset(DataWriter &out) const;

	// Change this condition to always be false.
	void MakeNever();

	bool operator==(const ConditionSet &other) const
	{
		if(isOr != other.isOr)
			return false;
		if(hasAssign != other.hasAssign)
			return false;
		if(expressions != other.expressions)
			return false;
		if(children != other.children)
			return false;
		return true;
	}
	bool operator!=(const ConditionSet &other) const { return !(*this == other); }

	// Check if there are any entries in this set.
	bool IsEmpty() const;

	// Check if this condition set contains valid data.
	bool IsValid() const;

	// Check if the given condition values satisfy this set of expressions.
	bool Test(const ConditionsStore &conditions) const;

	// Evaluate this expression into a numerical value. (The value can also be used as boolean.)
	int64_t Evaluate(const ConditionsStore &conditionsStore) const;

	/// Parse the remainder of a node into this expression.
	bool ParseNode(const DataNode &node, int &tokenNr);

	/// Optimize this node, this optimization also removes intermediate sections that were used for tracking brackets.
	bool Optimize(const DataNode &node);

	// Get the names of the conditions that are modified by this ConditionSet.
	std::set<std::string> RelevantConditions() const;


private:
<<<<<<< HEAD
	/// Parse a node completely into this expression; all tokens on the line and all children if there are any.
	bool ParseNode(const DataNode &node);

	/// Parse the children under 'and'-nodes, 'or'-nodes, or the toplevel-node (which acts as and-node). The
	/// expression-operator should already have been set before calling this function.
	bool ParseBooleanChildren(const DataNode &node);

	/// Parse a minimal complete expression from the tokens into the (empty) expression.
	///
	/// @param node The node to report parse-errors on, if any occur.
	/// @param lineTokens Tokens to use (and pop from) for parsing.
	bool ParseMini(const DataNode &node, int &tokenNr);

	/// Helper function to parse an infix operator and the subexpression after it.
	/// Should be called on ConditionSets that already have at least 1 sub-expression in them.
	///
	/// @param node The node to report parse-errors on, if any occur.
	/// @param lineTokens Tokens to use (and pop from) for parsing.
	bool ParseFromInfix(const DataNode &node, int &tokenNr, ExpressionOp parentOp);

	/// Push sub-expressions and the operator from the current expression one level down into a new single
	/// sub-expression.
	///
	/// To be used if the current expressions has precedence over the next infix operator that we are about to parse.
	/// @param node Node on which to report the failures (using node.PrintTrace()).
	bool PushDownFull(const DataNode &node);

	/// Push the last sub-expression from the current expression one level down into a new sub-expression.
	///
	/// To be used when the next infix-operator has precedence over the current operators being processed.
	/// @param node Node on which to report the failures (using node.PrintTrace()).
	bool PushDownLast(const DataNode &node);

	/// Handles a failure in parsing of lower-level nodes, for higher-level nodes;
	/// - Clears the sub-expressions and sets the operator to INVALID.
	bool FailParse();

	/// Handles a failure in parsing;
	/// - Reports the failure using PrintTrace() in the given DataNode.
	/// - Clears the sub-expressions and sets the operator to INVALID.
	///
	/// @param node Node on which to report the failures (using node.PrintTrace()).
	/// @param failText The reason why parsing is failing. (Will be used as output for node.PrintTrace()).
	/// @return false So that is can be used as a one-liner for failures.
	bool FailParse(const DataNode &node, const std::string &failText);
=======
	// This class represents a single expression involving a condition,
	// either testing what value it has, or modifying it in some way.
	class Expression {
	public:
		Expression(const std::vector<std::string> &left, const std::string &op, const std::vector<std::string> &right);
		Expression(const std::string &left, const std::string &op, const std::string &right);

		void Save(DataWriter &out) const;

		bool operator==(const Expression &other) const
		{
			if(op != other.op)
				return false;
			if(fun != other.fun)
				return false;
			if(left != other.left)
				return false;
			if(right != other.right)
				return false;
			return true;
		}
		bool operator!=(const Expression &other) const { return !(*this == other); }

		// Convert this expression into a string, for traces.
		std::string ToString() const;

		// Determine if this Expression instantiated properly.
		bool IsEmpty() const;

		// Returns the left side of this Expression.
		std::string Name() const;
		// True if this Expression performs a comparison and false if it performs an assignment.
		bool IsTestable() const;

		// Functions to use this expression:
		bool Test(const ConditionsStore &conditions, const ConditionsStore &created) const;
		void Apply(ConditionsStore &conditions, ConditionsStore &created) const;
		void TestApply(const ConditionsStore &conditions, ConditionsStore &created) const;


	private:
		// A SubExpression results from applying operator-precedence parsing to one side of
		// an Expression. The operators and tokens needed to recreate the given side are
		// stored, and can be interleaved to restore the original string. Based on them, a
		// sequence of "Operations" is created for runtime evaluation.
		class SubExpression {
		public:
			explicit SubExpression(const std::vector<std::string> &side);
			explicit SubExpression(const std::string &side);

			bool operator==(const SubExpression &other) const
			{
				if(sequence != other.sequence)
					return false;
				if(tokens != other.tokens)
					return false;
				if(operators != other.operators)
					return false;
				if(operatorCount != other.operatorCount)
					return false;
				return true;
			}
			bool operator!=(const SubExpression &other) const { return !(*this == other); }

			// Interleave tokens and operators to reproduce the initial string.
			const std::string ToString() const;
			// Interleave tokens and operators, but do not combine.
			const std::vector<std::string> ToStrings() const;

			bool IsEmpty() const;

			// Substitute numbers for any string values and then compute the result.
			int64_t Evaluate(const ConditionsStore &conditions, const ConditionsStore &created) const;


		private:
			void ParseSide(const std::vector<std::string> &side);
			void GenerateSequence();
			bool AddOperation(std::vector<int> &data, size_t &index, const size_t &opIndex);


		private:
			// An Operation has a pointer to its binary function, and the data indices for
			// its operands. The result is always placed on the back of the data vector.
			class Operation {
			public:
				explicit Operation(const std::string &op, size_t &a, size_t &b);

				bool operator==(const Operation &other) const
				{
					if(fun != other.fun)
						return false;
					if(a != other.a)
						return false;
					if(b != other.b)
						return false;
					return true;
				}
				bool operator!=(const Operation &other) const { return !(*this == other); }

				int64_t (*fun)(int64_t, int64_t);
				size_t a;
				size_t b;
			};


		private:
			// Iteration of the sequence vector yields the result.
			std::vector<Operation> sequence;
			// The tokens vector converts into a data vector of numeric values during evaluation.
			std::vector<std::string> tokens;
			std::vector<std::string> operators;
			// The number of true (non-parentheses) operators.
			int operatorCount = 0;
		};


	private:
		// String representation of the Expression's binary function.
		std::string op;
		// Pointer to a binary function that defines the assignment or
		// comparison operation to be performed between SubExpressions.
		int64_t (*fun)(int64_t, int64_t);

		// SubExpressions contain one or more tokens and any number of simple operators.
		SubExpression left;
		SubExpression right;
	};
>>>>>>> 0.10.10-editor-patched


private:
	/// Sets of condition tests can contain nested sets of tests. Each set is
	/// combined using the expression operator that determines how the nested
	/// sets are to be combined.
	/// Using an `and`-operator with no sub-expressions as safe initial value.
	ExpressionOp expressionOperator = ExpressionOp::AND;
	/// Literal part of the expression, if this is a literal terminal.
	int64_t literal = 0;
	/// Condition variable that is used in this expression, if this is a condition variable.
	std::string conditionName;
	/// Nested sets of conditions to be tested.
	std::vector<ConditionSet> children;

	// Let the assignment class call internal functions and parsers.
	friend class ConditionAssignments;
};
