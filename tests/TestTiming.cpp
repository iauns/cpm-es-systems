
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
  static int32_t numExecutions;

  void execute(es::ESCoreBase&, uint64_t entityID,
               const CompPosition* pos, const CompHomPos* homPos,
               const CompGameplay* gp) override
  {
    // Check the values contained in each of pos, homPos, and gp.
    pos->checkEqual(posComponents[entityID]);
    homPos->checkEqual(homPosComponents[entityID]);
    gp->checkEqual(gameplayComponents[entityID]);
    ++numExecutions;
  }

  // Compile time polymorphic function required by CerealCore when registering.
  static const char* getName()
  {
    return "ren:b";
  }
};
int32_t BasicSystem::numExecutions = 0;

class SystemOne : public es::GenericSystem<false, CompHomPos, CompGameplay>
{
public:
  static int32_t numExecutions;

  void execute(es::ESCoreBase&, uint64_t entityID,
               const CompHomPos* homPos,
               const CompGameplay* gp) override
  {
    homPos->checkEqual(homPosComponents[entityID]);
    gp->checkEqual(gameplayComponents[entityID]);
    ++numExecutions;
  }
  
  // Compile time polymorphic function required by CerealCore when registering.
  static const char* getName() {return "ren:a";}
};
int32_t SystemOne::numExecutions = 0;

class A : public es::GenericSystem<false, CompPosition>
{
public:
  static int32_t numExecutions;

  void execute(es::ESCoreBase&, uint64_t entityID, const CompPosition* pos) override
  {
    pos->checkEqual(posComponents[entityID]);
    ++numExecutions;
  }
  
  // Compile time polymorphic function required by CerealCore when registering.
  static const char* getName()
  {
    return "A";
  }
};
int32_t A::numExecutions = 0;

class Alower : public es::GenericSystem<false, CompHomPos>
{
public:
  static int32_t numExecutions;

  void execute(es::ESCoreBase&, uint64_t entityID, const CompHomPos* pos) override
  {
    pos->checkEqual(homPosComponents[entityID]);
    ++numExecutions;
  }
  
  // Compile time polymorphic function required by CerealCore when registering.
  static const char* getName()
  {
    return "a";
  }
};
int32_t Alower::numExecutions = 0;

class B : public es::GenericSystem<false, CompGameplay>
{
public:
  static int32_t numExecutions;

  void execute(es::ESCoreBase&, uint64_t entityID, const CompGameplay* gp) override
  {
    gp->checkEqual(gameplayComponents[entityID]);
    ++numExecutions;
  }
  
  // Compile time polymorphic function required by CerealCore when registering.
  static const char* getName()
  {
    return "B";
  }
};
int32_t B::numExecutions = 0;

class C : public es::GenericSystem<false, CompPosition, CompGameplay>
{
public:
  static int32_t numExecutions;

  void execute(es::ESCoreBase&, uint64_t entityID, const CompPosition* pos, const CompGameplay* gp) override
  {
    pos->checkEqual(posComponents[entityID]);
    gp->checkEqual(gameplayComponents[entityID]);
    ++numExecutions;
  }
  
  // Compile time polymorphic function required by CerealCore when registering.
  static const char* getName()
  {
    return "C";
  }
};
int32_t C::numExecutions = 0;

TEST(EntitySystem, TimingTest)
{
  esys::SystemCore::clearRegisteredSystems();

  // Generate entity system core.
  std::shared_ptr<es::ESCore> core(new es::ESCore());
  std::shared_ptr<esys::SystemCore> systems(new esys::SystemCore);

  // Register systems. All of the above text regarding components applies
  // to systems as well.
  systems->registerSystem<SystemOne>();
  systems->registerSystem<B>();
  systems->registerSystem<Alower>();
  systems->registerSystem<BasicSystem>();
  systems->registerSystem<C>();
  systems->registerSystem<A>();

  // Destroy the core and re-register components and systems.
  // Then deserialize the data from memory to see if the correct components
  // and systems are serialized back in.

  uint64_t rootID = core->getNewEntityID();
  uint64_t id = rootID;
  core->addComponent(id, posComponents[id]);
  core->addComponent(id, homPosComponents[id]);
  core->addComponent(id, gameplayComponents[id]);

  core->renormalize(true);

  systems->addActiveSystemViaType<C>(3, 0);
  systems->addActiveSystemViaType<Alower>(4, 5);          // First execution should be on 8.
  systems->addActiveSystemViaType<SystemOne>(4, 10, 1);   // First execution should be on 11.
  systems->addActiveSystemViaType<B>();
  systems->addActiveSystemViaType<BasicSystem>(5, 10, 1); // First execution should be on 14.
  systems->addActiveSystemViaType<A>(1, 0);
  systems->renormalize();

  // Run the systems and tests which tests were run.
  systems->runSystems(*core, 0);
  EXPECT_EQ(1, A::numExecutions);
  EXPECT_EQ(1, B::numExecutions);
  EXPECT_EQ(1, C::numExecutions);
  EXPECT_EQ(0, Alower::numExecutions);
  EXPECT_EQ(0, BasicSystem::numExecutions);
  EXPECT_EQ(0, SystemOne::numExecutions);

  systems->runSystems(*core, 1);
  EXPECT_EQ(2, A::numExecutions);
  EXPECT_EQ(2, B::numExecutions);
  EXPECT_EQ(1, C::numExecutions);
  EXPECT_EQ(0, Alower::numExecutions);
  EXPECT_EQ(0, BasicSystem::numExecutions);
  EXPECT_EQ(0, SystemOne::numExecutions);

  systems->runSystems(*core, 2);
  EXPECT_EQ(3, A::numExecutions);
  EXPECT_EQ(3, B::numExecutions);
  EXPECT_EQ(1, C::numExecutions);
  EXPECT_EQ(0, Alower::numExecutions);
  EXPECT_EQ(0, BasicSystem::numExecutions);
  EXPECT_EQ(0, SystemOne::numExecutions);

  systems->runSystems(*core, 3);
  EXPECT_EQ(4, A::numExecutions);
  EXPECT_EQ(4, B::numExecutions);
  EXPECT_EQ(2, C::numExecutions);
  EXPECT_EQ(0, Alower::numExecutions);
  EXPECT_EQ(0, BasicSystem::numExecutions);
  EXPECT_EQ(0, SystemOne::numExecutions);

  systems->runSystems(*core, 4);
  EXPECT_EQ(5, A::numExecutions);
  EXPECT_EQ(5, B::numExecutions);
  EXPECT_EQ(2, C::numExecutions);
  EXPECT_EQ(0, Alower::numExecutions);
  EXPECT_EQ(0, BasicSystem::numExecutions);
  EXPECT_EQ(0, SystemOne::numExecutions);

  // Re-execute the same time. Only system be should re-execute since it has
  // no interval.
  systems->runSystems(*core, 4);
  EXPECT_EQ(5, A::numExecutions);
  EXPECT_EQ(6, B::numExecutions);
  EXPECT_EQ(2, C::numExecutions);
  EXPECT_EQ(0, Alower::numExecutions);
  EXPECT_EQ(0, BasicSystem::numExecutions);
  EXPECT_EQ(0, SystemOne::numExecutions);

  systems->runSystems(*core, 5);
  EXPECT_EQ(6, A::numExecutions);
  EXPECT_EQ(7, B::numExecutions);
  EXPECT_EQ(2, C::numExecutions);
  EXPECT_EQ(0, Alower::numExecutions);
  EXPECT_EQ(0, BasicSystem::numExecutions);
  EXPECT_EQ(0, SystemOne::numExecutions);

  systems->runSystems(*core, 6);
  EXPECT_EQ(7, A::numExecutions);
  EXPECT_EQ(8, B::numExecutions);
  EXPECT_EQ(3, C::numExecutions);
  EXPECT_EQ(0, Alower::numExecutions);
  EXPECT_EQ(0, BasicSystem::numExecutions);
  EXPECT_EQ(0, SystemOne::numExecutions);

  systems->runSystems(*core, 7);
  EXPECT_EQ(8, A::numExecutions);
  EXPECT_EQ(9, B::numExecutions);
  EXPECT_EQ(3, C::numExecutions);
  EXPECT_EQ(0, Alower::numExecutions);
  EXPECT_EQ(0, BasicSystem::numExecutions);
  EXPECT_EQ(0, SystemOne::numExecutions);

  systems->runSystems(*core, 8);
  EXPECT_EQ(9,  A::numExecutions);
  EXPECT_EQ(10, B::numExecutions);
  EXPECT_EQ(3,  C::numExecutions);
  EXPECT_EQ(1,  Alower::numExecutions);
  EXPECT_EQ(0,  BasicSystem::numExecutions);
  EXPECT_EQ(0,  SystemOne::numExecutions);

  systems->runSystems(*core, 9);
  EXPECT_EQ(10, A::numExecutions);
  EXPECT_EQ(11, B::numExecutions);
  EXPECT_EQ(4,  C::numExecutions);
  EXPECT_EQ(1,  Alower::numExecutions);
  EXPECT_EQ(0,  BasicSystem::numExecutions);
  EXPECT_EQ(0,  SystemOne::numExecutions);

  systems->runSystems(*core, 10);
  EXPECT_EQ(11, A::numExecutions);
  EXPECT_EQ(12, B::numExecutions);
  EXPECT_EQ(4,  C::numExecutions);
  EXPECT_EQ(1,  Alower::numExecutions);
  EXPECT_EQ(0,  BasicSystem::numExecutions);
  EXPECT_EQ(0,  SystemOne::numExecutions);

  systems->runSystems(*core, 11);
  EXPECT_EQ(12, A::numExecutions);
  EXPECT_EQ(13, B::numExecutions);
  EXPECT_EQ(4,  C::numExecutions);
  EXPECT_EQ(1,  Alower::numExecutions);
  EXPECT_EQ(0,  BasicSystem::numExecutions);
  EXPECT_EQ(1,  SystemOne::numExecutions);

  systems->runSystems(*core, 12);
  EXPECT_EQ(13, A::numExecutions);
  EXPECT_EQ(14, B::numExecutions);
  EXPECT_EQ(5,  C::numExecutions);
  EXPECT_EQ(2,  Alower::numExecutions);
  EXPECT_EQ(0,  BasicSystem::numExecutions);
  EXPECT_EQ(1,  SystemOne::numExecutions);

  systems->runSystems(*core, 13);
  EXPECT_EQ(14, A::numExecutions);
  EXPECT_EQ(15, B::numExecutions);
  EXPECT_EQ(5,  C::numExecutions);
  EXPECT_EQ(2,  Alower::numExecutions);
  EXPECT_EQ(0,  BasicSystem::numExecutions);
  EXPECT_EQ(1,  SystemOne::numExecutions);

  systems->runSystems(*core, 14);
  EXPECT_EQ(15, A::numExecutions);
  EXPECT_EQ(16, B::numExecutions);
  EXPECT_EQ(5,  C::numExecutions);
  EXPECT_EQ(2,  Alower::numExecutions);
  EXPECT_EQ(1,  BasicSystem::numExecutions);
  EXPECT_EQ(1,  SystemOne::numExecutions);

  systems->runSystems(*core, 15);
  EXPECT_EQ(16, A::numExecutions);
  EXPECT_EQ(17, B::numExecutions);
  EXPECT_EQ(6,  C::numExecutions);
  EXPECT_EQ(2,  Alower::numExecutions);
  EXPECT_EQ(1,  BasicSystem::numExecutions);
  EXPECT_EQ(2,  SystemOne::numExecutions);

  systems->runSystems(*core, 16);
  EXPECT_EQ(17, A::numExecutions);
  EXPECT_EQ(18, B::numExecutions);
  EXPECT_EQ(6,  C::numExecutions);
  EXPECT_EQ(3,  Alower::numExecutions);
  EXPECT_EQ(1,  BasicSystem::numExecutions);
  EXPECT_EQ(2,  SystemOne::numExecutions);

  systems->runSystems(*core, 17);
  EXPECT_EQ(18, A::numExecutions);
  EXPECT_EQ(19, B::numExecutions);
  EXPECT_EQ(6,  C::numExecutions);
  EXPECT_EQ(3,  Alower::numExecutions);
  EXPECT_EQ(1,  BasicSystem::numExecutions);
  EXPECT_EQ(2,  SystemOne::numExecutions);

  systems->runSystems(*core, 18);
  EXPECT_EQ(19, A::numExecutions);
  EXPECT_EQ(20, B::numExecutions);
  EXPECT_EQ(7,  C::numExecutions);
  EXPECT_EQ(3,  Alower::numExecutions);
  EXPECT_EQ(1,  BasicSystem::numExecutions);
  EXPECT_EQ(2,  SystemOne::numExecutions);

  systems->runSystems(*core, 19);
  EXPECT_EQ(20, A::numExecutions);
  EXPECT_EQ(21, B::numExecutions);
  EXPECT_EQ(7,  C::numExecutions);
  EXPECT_EQ(3,  Alower::numExecutions);
  EXPECT_EQ(2,  BasicSystem::numExecutions);
  EXPECT_EQ(3,  SystemOne::numExecutions);
}

}

