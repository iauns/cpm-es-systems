
#include <entity-system/GenericSystem.hpp>
#include <entity-system/ESCore.hpp>
#include <es-systems/SystemCore.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <glm/glm.hpp>

namespace es = CPM_ES_NS;
namespace esys = CPM_ES_SYSTEMS_NS;

namespace {

// We may want to enforce that these components have bson serialization members
// (possibly a static assert?).

struct CompPosition
{
  CompPosition() {}
  CompPosition(const glm::vec3& pos) {position = pos;}

  void checkEqual(const CompPosition& pos) const
  {
    EXPECT_FLOAT_EQ(position.x, pos.position.x);
    EXPECT_FLOAT_EQ(position.y, pos.position.y);
    EXPECT_FLOAT_EQ(position.z, pos.position.z);
  }

  // What this 'struct' is all about -- the data.
  glm::vec3 position;
};

struct CompHomPos
{
  CompHomPos() {}
  CompHomPos(const glm::vec4& pos) {position = pos;}

  void checkEqual(const CompHomPos& pos) const
  {
    EXPECT_FLOAT_EQ(position.x, pos.position.x);
    EXPECT_FLOAT_EQ(position.y, pos.position.y);
    EXPECT_FLOAT_EQ(position.z, pos.position.z);
    EXPECT_FLOAT_EQ(position.w, pos.position.w);
  }

  // DATA
  glm::vec4 position;
};

struct CompGameplay
{
  CompGameplay() : health(0), armor(0) {}
  CompGameplay(int healthIn, int armorIn)
  {
    this->health = healthIn;
    this->armor = armorIn;
  }

  void checkEqual(const CompGameplay& gp) const
  {
    EXPECT_EQ(health, gp.health);
    EXPECT_EQ(armor, gp.armor);
  }

  // DATA
  int32_t health;
  int32_t armor;
};

// Component positions. associated with id. The first component is not used.
std::vector<CompPosition> posComponents = {
  glm::vec3(0.0, 0.0, 0.0),
  glm::vec3(1.0, 2.0, 3.0),
  glm::vec3(5.5, 6.0, 10.7),
  glm::vec3(1.5, 3.0, 107),
  glm::vec3(4.0, 7.0, 9.0),
  glm::vec3(2.92, 89.0, 4.0),
};

std::vector<CompHomPos> homPosComponents = {
  glm::vec4(0.0, 0.0, 0.0, 0.0),
  glm::vec4(1.0, 11.0, 41.0, 51.0),
  glm::vec4(2.0, 12.0, 42.0, 52.0),
  glm::vec4(3.0, 13.0, 43.0, 53.0),
  glm::vec4(4.0, 14.0, 44.0, 54.0),
  glm::vec4(5.0, 15.0, 45.0, 55.0),
};

std::vector<CompGameplay> gameplayComponents = {
  CompGameplay(0, 0),
  CompGameplay(45, 21),
  CompGameplay(23, 123),
  CompGameplay(99, 892),
  CompGameplay(73, 64),
  CompGameplay(23, 92),
};

// This basic system will apply, every frame, to entities with the CompPosition,
// CompHomPos, and CompGameplay components.
class BasicSystem : public es::GenericSystem<false, CompPosition, CompHomPos, CompGameplay>
{
public:

  static std::map<uint64_t, bool> invalidComponents;
  static int32_t runCount;

  void execute(es::ESCoreBase&, uint64_t entityID,
               const CompPosition* pos, const CompHomPos* homPos,
               const CompGameplay* gp) override
  {
    // Check to see if this entityID should have been executed.
    if (invalidComponents.find(entityID) != invalidComponents.end())
      FAIL() << "BasicSystem attempt to execute on an invalid component." << std::endl;

    // Check the values contained in each of pos, homPos, and gp.
    pos->checkEqual(posComponents[entityID]);
    homPos->checkEqual(homPosComponents[entityID]);
    gp->checkEqual(gameplayComponents[entityID]);

    ++runCount;
  }

  // Compile time polymorphic function required by CerealCore when registering.
  static const char* getName()
  {
    return "ren:BasicSystem";
  }
};
int32_t BasicSystem::runCount = 0;

class SystemOne : public es::GenericSystem<false, CompHomPos, CompGameplay>
{
public:
  static std::map<uint64_t, bool> invalidComponents;
  static int32_t runCount;

  void execute(es::ESCoreBase&, uint64_t entityID,
               const CompHomPos* homPos,
               const CompGameplay* gp) override
  {
    // Basic system should run before SystemOne because of alphabetical order.
    EXPECT_GT(BasicSystem::runCount, 0);

    // Check to see if this entityID should have been executed.
    if (invalidComponents.find(entityID) != invalidComponents.end())
      FAIL() << "SystemOne attempt to execute on an invalid component." << std::endl;

    // Check the values contained in each of pos, homPos, and gp.
    homPos->checkEqual(homPosComponents[entityID]);
    gp->checkEqual(gameplayComponents[entityID]);

    ++runCount;
  }
  
  // Compile time polymorphic function required by CerealCore when registering.
  static const char* getName()
  {
    return "ren:SystemOne";
  }
};
int32_t SystemOne::runCount = 0;

std::map<uint64_t, bool> BasicSystem::invalidComponents;
std::map<uint64_t, bool> SystemOne::invalidComponents;

TEST(EntitySystem, BasicConstruction)
{
  // Generate entity system core.
  std::shared_ptr<es::ESCore> core(new es::ESCore());
  std::shared_ptr<esys::SystemCore> systems(new esys::SystemCore);

  systems->clearRegisteredSystems();

  // Register systems. All of the above text regarding components applies
  // to systems as well.
  systems->registerSystem<SystemOne>();
  systems->registerSystem<BasicSystem>();

  // Destroy the core and re-register components and systems.
  // Then deserialize the data from memory to see if the correct components
  // and systems are serialized back in.

  uint64_t rootID = core->getNewEntityID();
  uint64_t id = rootID;
  core->addComponent(id, posComponents[id]);
  core->addComponent(id, homPosComponents[id]);
  core->addComponent(id, gameplayComponents[id]);

  id = core->getNewEntityID();
  core->addComponent(id, homPosComponents[id]);
  core->addComponent(id, gameplayComponents[id]);
  BasicSystem::invalidComponents.insert(std::make_pair(id, true));

  id = core->getNewEntityID();
  core->addComponent(id, posComponents[id]);
  core->addComponent(id, homPosComponents[id]);
  core->addComponent(id, gameplayComponents[id]);

  id = core->getNewEntityID();
  core->addComponent(id, posComponents[id]);
  core->addComponent(id, homPosComponents[id]);
  core->addComponent(id, gameplayComponents[id]);
  
  std::shared_ptr<BasicSystem> sysBasic(new BasicSystem());
  std::shared_ptr<SystemOne> sysOne(new SystemOne());

  core->renormalize(true);

  systems->addActiveSystemViaType<SystemOne>();
  systems->addActiveSystemViaType<BasicSystem>();
  systems->renormalize();

  systems->runSystems(*core, 0);
  systems->runSystems(*core, 0);

  EXPECT_EQ(2 * 3, BasicSystem::runCount);
  EXPECT_EQ(2 * 4, SystemOne::runCount);
}

}

