#include "policyDecision.h"

bool PolicyDecisionPoint::EvaluateDecision(std::string role, Resource_Type resource)
{
	std::pair<std::string, Resource_Type> policy = std::make_pair(role, resource);
	std::vector<std::pair<std::string, Resource_Type>>::iterator result = std::find(policies.begin(), policies.end(), policy);

	if(result != policies.end())
	{
		return true;
	}

	return false;
}