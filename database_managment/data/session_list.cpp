#include "session_list.h"

SessionList::SessionList() : version(0)
{
}

SessionList::SessionList(sessionListVersionType version) : version(version)
{
}

SessionList::~SessionList()
{
}
