#include "Parser.h"
#include "Experimenting.h"


Experimenting::Experimenting(Parser * par, list<Path *> * pat, char dir) :
  parent(par), direction(dir), shortPaths(pat),
  paths(new list<Path *>), best(0), second(0), numPaths(0)
  {}




void Experimenting::receiveRoom(RoomAdmin * map, Room * room)
{

  for (list<Path *>::iterator i = shortPaths->begin(); i != shortPaths->end(); ++i)
  {
    Coordinate * c = parent->getExpectedCoordinate((*i)->getRoom());
    Path * working = (*i)->fork(room, c, map, params, this, direction);
    if (best == 0) best = working;
    else if (working->getProb() > best->getProb())
    {
      paths->push_back(best);
      second = best;
      best = working;
    }
    else
    {
      if (second == 0 || working->getProb() > second->getProb())
        second = working;
      paths->push_back(working);
    }

    cmm.deactivate(c);
    numPaths++;
  }
}


list<Path *> * Experimenting::evaluate()
{
  Path * working = 0;
  while(!shortPaths->empty())
  {
    working = shortPaths->front();
    shortPaths->pop_front();
    if (!(working->hasChildren())) working->deny();
  }

  if (best != 0)
  {
    if (second == 0 || best->getProb() > second->getProb()*params.acceptBest)
    {
      for (list<Path *>::iterator i = paths->begin(); i != paths->end(); ++i)
      {
        (*i)->deny();
      }
      paths->clear();
      paths->push_front(best);
    }
    else
    {
      paths->push_back(best);
      for (Path * working = 0; (working = paths->front()) != best; paths->pop_front())
      {
        if ( best->getProb() > working->getProb()*params.maxPaths/numPaths)
        {
          working->deny();
        }
        else paths->push_back(working);
      }
    }
  }
  second = 0;
  delete shortPaths;
  shortPaths = 0;
  best = 0;
  return paths;
}

