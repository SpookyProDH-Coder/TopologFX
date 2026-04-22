#include "Registry.h"

static Registry g_registry;

Registry& GetRegistry()
{
	return g_registry;
}