#ifndef ROOMADMIN
#define ROOMADMIN

#include <qthread.h>
#include <qmutex.h>
#include <vector>
#include <stack>
#include <set>
#include <map>
#include <qmap.h>

#include "RoomSearchNode.h"
#include "IntermediateNode.h"
#include "Component.h"
#include "Frustum.h"
#include "Room.h"
#include "Map.h"





using namespace std;


/**
 * The RoomAdmin organizes rooms and their relations to each other.
 */
class RoomAdmin : public Component, public IntermediateNode {
 public:
  RoomAdmin();
  virtual Qt::ConnectionType requiredConnectionType(const QString &) {return Qt::DirectConnection;}

 private:
  Q_OBJECT

  Map map;
  vector<Room *> roomIndex;
  vector<set<QObject *> > locks;
  stack<int>  unusedIds;
  unsigned int greatestUsedId;
  QMutex mapLock;


  void removeRoom(int id); 
  void assignId(Room * room); 
  unsigned int lastId() {return greatestUsedId;}

 public slots:
  // looking for rooms leads to a bunch of foundRoom() signals
  void lookingForRooms(QObject *,ParseEvent *);
  void lookingForRooms(QObject *,unsigned int); // by id
  void lookingForRooms(QObject *,Coordinate *);
  void lookingForRooms(QObject *,Frustum *);

  // createRoom creates a room without a lock
  // it will get deleted if no one looks for it for a certain time
  void createRoom(ParseEvent *, Coordinate *, char);

  // same as above, but creates an anonymous lock on the room 
  // and tries to assign a special id
  void createPredefinedRoom(ParseEvent *, Coordinate *, char, int);

  // addExit doesn't lock a room either
  void addExit(int, int, int);

  // removes the lock on a room
  // after the last lock is removed, the room is deleted
  void releaseRoom(QObject *, int);

  // makes a lock on a room permanent and anonymous.
  // Like that the room can't be deleted via releaseRoom anymore.
  void keepRoom(QObject *, int);

  // other slots like moveRoom and forceDeleteRoom need to be 
  // implemented later

 signals:
  // if a room is sent out via foundRoom it is locked so that it doesn't
  // get deleted while being processed by another component.
  // a room can be locked by multiple components but each component can only create one lock
  void foundRoom(QObject *, Room *);

  // this signal is sent out if a room is deleted. So any clients still
  // working on this room can start some emergency action.
  void deletedRoom(QObject *, int);
};



	

#ifdef DMALLOC
#include <mpatrol.h>
#endif
#endif
