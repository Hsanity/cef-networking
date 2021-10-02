#include "policyEnforcement.h"

PolicyEnforcementPoint::PolicyEnforcementPoint()
{
	decisionPoint = PolicyDecisionPoint();
}

PolicyEnforcementPoint::~PolicyEnforcementPoint()
{
}

bool PolicyEnforcementPoint::RequestDecision(std::string query)
{
	return EnforcePolicy(query);
}

bool PolicyEnforcementPoint::EnforcePolicy(std::string query)
{
	//Authenticate
	Authenticator::getInstance()->SessionAlive();
	Resource_Type rt = GetResourceType(query.c_str());

	std::string role = Authenticator::getInstance()->getRole();

	return decisionPoint.EvaluateDecision("Potato Buns", rt);
}