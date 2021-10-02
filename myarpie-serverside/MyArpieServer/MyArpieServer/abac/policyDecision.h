#pragma once
#include "../authentication/authenticator.h"
#include <utility>
#include <algorithm>

class PolicyDecisionPoint
{
public:
	PolicyDecisionPoint(){};
	~PolicyDecisionPoint(){};

	bool EvaluateDecision(std::string role, Resource_Type resource);
private:
	std::vector<std::pair<std::string, Resource_Type>> policies = {
		std::make_pair<std::string, Resource_Type>
		("Potato Buns", Resource_Type::ACC_R),

		std::make_pair<std::string, Resource_Type>
		("Potato Buns", Resource_Type::ACC_W)
	};
};