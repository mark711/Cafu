/********************************/
/*** Info Player Start (Code) ***/
/********************************/

#include <string.h>
#include "InfoPlayerStart.hpp"
#include "EntityCreateParams.hpp"

using namespace GAME_NAME;


EntInfoPlayerStartT::EntInfoPlayerStartT(char TypeID, unsigned long ID, unsigned long MapFileID, cf::GameSys::GameWorldI* GameWorld, const VectorT& Origin)
    : BaseEntityT(EntityCreateParamsT(ID, std::map<std::string, std::string>(), NULL, NULL, MapFileID, GameWorld, Origin),
                  BoundingBox3dT(Vector3dT( 100.0,  100.0,  100.0),
                                 Vector3dT(-100.0, -100.0, -100.0)),
                  0)
{
}
