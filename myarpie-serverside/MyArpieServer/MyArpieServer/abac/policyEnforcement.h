#pragma once
#include <string>

#include "policyDecision.h"

class PolicyEnforcementPoint
{
public:
	PolicyEnforcementPoint();
	~PolicyEnforcementPoint();

	bool RequestDecision(std::string query);
	bool EnforcePolicy(std::string query);

private:
	PolicyDecisionPoint decisionPoint;

};