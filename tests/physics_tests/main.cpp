#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Core.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/Core/IssueReporting.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/EActivation.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/SubShapeID.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

#include "core/defines.hpp"
#include "core/logger.hpp"
#include "core/errors.hpp"

#include "parsing/cmd_line_parser.hpp"
#include "parsing/ini_parser.hpp"

static void TraceImpl(const char* in_fmt , ...);
static bool AssertFailedImpl(const char* expression , const char* message , const char* file , JPH::uint line);

constexpr static uint32_t kMaxBodies = 1000;

class BroadPhaseLayer final : public JPH::BroadPhaseLayerInterface {
  public:
    BroadPhaseLayer() {}
    virtual ~BroadPhaseLayer() override {}

    JPH::uint GetNumBroadPhaseLayers() const override {
      return 1;
    }

    JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override {
      return broad_phase_layer;
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char* GetBroadPhaseLayerName(JPH::BroadBroadPhaseLayer layer) const override {
      return "Debug Broad Phase Layer";
    }
#endif // !JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

  private:
    JPH::BroadPhaseLayer broad_phase_layer;
};

class ObjectVsBroadPhaseLayerFilter final : public JPH::ObjectVsBroadPhaseLayerFilter {
  public:
    virtual ~ObjectVsBroadPhaseLayerFilter() override {}

    bool ShouldCollide(JPH::ObjectLayer layer1 , JPH::BroadPhaseLayer layer2) const override {
      return true;
    }
};

class ObjectLayerPairFilter final : public JPH::ObjectLayerPairFilter {
  public:
    virtual ~ObjectLayerPairFilter() override {} 

    bool ShouldCollide(JPH::ObjectLayer layer1 , JPH::ObjectLayer layer2) const override {
      return true;
    }
};

class ActivationListener final : public JPH::BodyActivationListener {
  public:
    virtual ~ActivationListener() override {}

    void OnBodyActivated(const JPH::BodyID& id , uint64_t body_user_data) override {
      OE_DEBUG("Body {} activated ({})" , id.GetIndex() , body_user_data);
    }

    void OnBodyDeactivated(const JPH::BodyID& id , uint64_t body_user_data) override {
      OE_DEBUG("Body {} deactivated ({})" , id.GetIndex() , body_user_data);
    }
};

class ContactListener final : public JPH::ContactListener {
  public:
    virtual ~ContactListener() override {}

    JPH::ValidateResult OnContactValidate(const JPH::Body& body1, const JPH::Body& body2, JPH::RVec3Arg base_offset , const JPH::CollideShapeResult& collision_result) override {
      OE_CHECK_AND_REGISTER_THREAD("PhysicsSystemThread");

      OE_DEBUG("OnContactValidate {} vs {}" , body1.GetID().GetIndex() , body2.GetID().GetIndex());
      return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
    }

    void OnContactAdded(const JPH::Body& body1 , const JPH::Body& body2 , const JPH::ContactManifold& manifold , JPH::ContactSettings& settings) override {
      OE_CHECK_AND_REGISTER_THREAD("PhysicsSystemThread");
      OE_DEBUG("OnContactAdded {} vs {}" , body1.GetID().GetIndex() , body2.GetID().GetIndex());
    }

    void OnContactPersisted(const JPH::Body& body1 , const JPH::Body& body2 , const JPH::ContactManifold& manifold , JPH::ContactSettings& settings) override {
      OE_CHECK_AND_REGISTER_THREAD("PhysicsSystemThread");
      OE_DEBUG("OnContactPersisted {} vs {}" , body1.GetID().GetIndex() , body2.GetID().GetIndex());
    }

    void OnContactRemoved(const JPH::SubShapeIDPair& sub_shape_pair) override {
      OE_CHECK_AND_REGISTER_THREAD("PhysicsSystemThread");
      OE_DEBUG("OnContactRemoved");
    }
};

int main(int argc , char* argv[]) {
  using namespace JPH::literals;

  uint32_t res = 0;
  try {
    other::CmdLine cmd_line(argc , argv);

    std::string config_path = "C:/Yock/code/OtherEngine/tests/physics_tests/physics.other";
    other::IniFileParser parser(config_path);
    auto config = parser.Parse();

    other::Logger::Open(config);
    other::Logger::Instance()->RegisterThread("Physics-Thread");

    /// Begin Jolt Code
    JPH::RegisterDefaultAllocator();
    JPH::Trace = TraceImpl;
    JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl);

    JPH::Factory::sInstance = new JPH::Factory();

    JPH::RegisterTypes();

    JPH::TempAllocatorImpl temp_allocator(10 * 1024 * 1024);
    JPH::JobSystemThreadPool job_system(JPH::cMaxPhysicsJobs , JPH::cMaxPhysicsBarriers , std::thread::hardware_concurrency() - 1);

    const JPH::uint max_bodies = 1024;
    const JPH::uint num_mutexes = 0;
    const JPH::uint max_pairs = 1024;
    const JPH::uint max_contact_constraints = 1024;

    BroadPhaseLayer broad_phase_layer;
    ObjectVsBroadPhaseLayerFilter obj_vs_broadphase_filter;
    ObjectLayerPairFilter obj_layer_pair_filter;

    OE_DEBUG("Initializing Physics System");
    JPH::PhysicsSystem physics_system;
    physics_system.Init(max_bodies, num_mutexes , max_pairs , max_contact_constraints , 
                        broad_phase_layer , obj_vs_broadphase_filter , obj_layer_pair_filter);

    ActivationListener activation_listener;
    physics_system.SetBodyActivationListener(&activation_listener);

    ContactListener contact_listener;
    physics_system.SetContactListener(&contact_listener);

    JPH::BodyInterface& body_interface = physics_system.GetBodyInterface();

    JPH::BoxShapeSettings floor_shape_settings(JPH::Vec3(100.f , 1.f , 100.f));
    floor_shape_settings.SetEmbedded();

    JPH::ShapeSettings::ShapeResult floor_shape_res = floor_shape_settings.Create();
    JPH::ShapeRefC floor_shape = floor_shape_res.Get();
    JPH::BodyCreationSettings floor_settings(floor_shape , JPH::RVec3(0.0_r , -1.0_r , 0.0_r) , 
                                             JPH::Quat::sIdentity() , JPH::EMotionType::Static , 0);

    OE_DEBUG("Adding Floor");
    JPH::Body* floor = body_interface.CreateBody(floor_settings);

    body_interface.AddBody(floor->GetID() , JPH::EActivation::DontActivate);

    OE_DEBUG("Adding Sphere");
    JPH::BodyCreationSettings sphere_settings(new JPH::SphereShape(0.5f) , JPH::RVec3(0.0_r , 10.0_r , 0.0_r) , 
                                              JPH::Quat::sIdentity() , JPH::EMotionType::Dynamic , 1);
    JPH::BodyID sphere_id = body_interface.CreateAndAddBody(sphere_settings , JPH::EActivation::Activate);

    body_interface.SetLinearVelocity(sphere_id , JPH::Vec3(0.f , -5.0f , 0.f));

    const float dt = 1.f / 60.f;

    physics_system.OptimizeBroadPhase();

    JPH::uint step = 0;
    while (body_interface.IsActive(sphere_id)) {
      ++step;

      JPH::RVec3 pos = body_interface.GetCenterOfMassPosition(sphere_id);
      JPH::Vec3 vel = body_interface.GetLinearVelocity(sphere_id);

      OE_DEBUG("[STEP {}] :: Pos : [{},{},{}] | Vel : [{},{},{}]" , step , pos.GetX() , pos.GetY() , pos.GetZ() , 
                                                                    vel.GetX() , vel.GetY() , vel.GetZ());

      const uint32_t collision_steps = 1;
      physics_system.Update(dt , collision_steps , &temp_allocator , &job_system);
    }

    OE_DEBUG("Shutting down!");
    body_interface.RemoveBody(sphere_id);
    body_interface.DestroyBody(sphere_id);

    body_interface.RemoveBody(floor->GetID());
    body_interface.DestroyBody(floor->GetID());

    JPH::UnregisterTypes();

    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
  } catch (const other::IniException& e) {
    OE_ERROR("Caught Ini Parsing error! {}" , e.what());
    res = 1;
  } catch (const std::exception& e) {
    OE_ERROR("Caught std::exception! {} " , e.what());
    res = 1;
  } catch (...) {
    OE_ERROR("Caught Unknown exception!");
    res = 1;
  }

  other::Logger::Shutdown();
  return res;
}

void TraceImpl(const char* in_fmt , ...) {
  va_list list;
  va_start(list , in_fmt);
  char buffer[1024];
  vsnprintf(buffer , sizeof(buffer) , in_fmt , list);
  va_end(list);

  OE_TRACE("Physics Trace : {}" , std::string{ buffer });
}

bool AssertFailedImpl(const char* expression , const char* message , const char* file , JPH::uint line) {
  OE_CRITICAL("{}:{} : ({}) {}" , file , line , expression , message == nullptr ? "" : message);
  return true;
}
