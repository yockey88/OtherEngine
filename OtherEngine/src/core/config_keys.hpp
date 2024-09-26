/**
 * \file core/config_keys.hpp
 **/
#ifndef OTHER_ENGINE_CONFIG_KEYS_HPP
#define OTHER_ENGINE_CONFIG_KEYS_HPP

#include <string_view>

#include "core/defines.hpp"

namespace other {

  /// section keys
  constexpr static std::string_view kProjectSection = "PROJECT";
  constexpr static uint64_t kProjectSectionHash = FNV(kProjectSection);
  
  constexpr static std::string_view kDebugSection = "DEBUG";
  constexpr static uint64_t kDebugSectionHash = FNV(kDebugSection);

  constexpr static std::string_view kWindowSection = "WINDOW";
  constexpr static uint64_t kWindowSectionHash = FNV(kWindowSection);
  
  constexpr static std::string_view kWindowFlagsSection = "WINDOW.FLAGS";
  constexpr static uint64_t kWindowFlagsSectionHash = FNV(kWindowSection);

  constexpr static std::string_view kRendererSection = "RENDERER";
  constexpr static uint64_t kRendererSectionHash = FNV(kRendererSection);

  constexpr static std::string_view kUiSection = "UI";
  constexpr static uint64_t kUiSectionHash = FNV(kUiSection);

  constexpr static std::string_view kLogSection = "LOG";
  constexpr static uint64_t kLogSectionHash = FNV(kLogSection);

  constexpr static std::string_view kScriptEngineSection = "SCRIPT-ENGINE";
  constexpr static uint64_t kScriptEngineSectionHash = FNV(kScriptEngineSection);

  constexpr static std::string_view kEditorSection = "EDITOR";
  constexpr static uint64_t kEditorSectionHash = FNV(kEditorSection);
  
  constexpr static std::string_view kCsModuleSection = "C#";
  constexpr static uint64_t kCsModuleSectionHash = FNV(kCsModuleSection);
  
  constexpr static std::string_view kLuaModuleSection = "LUA";
  constexpr static uint64_t kLuaModuleSectionHash = FNV(kLuaModuleSection);

  constexpr static std::string_view kResourcesSection = "RESOURCES";
  constexpr static uint64_t kResourcesSectionHash = FNV(kResourcesSection);
  
  constexpr static std::string_view kMetadataSection = "METADATA";
  constexpr static uint64_t kMetadataSectionHash = FNV(kMetadataSection);

  constexpr static std::string_view kTransformSection = "TRANSFORM";
  constexpr static uint64_t kTransformSectionHash = FNV(kTransformSection);

  constexpr static std::string_view kRelationshipSection = "RELATIONSHIP";
  constexpr static uint64_t kRelationshipSectionHash = FNV(kRelationshipSection);

  /// useful value keys
  constexpr static std::string_view kNameValue = "NAME";
  constexpr static uint64_t kNameValueHash = FNV(kNameValue);
  
  constexpr static std::string_view kBinDirValue = "BIN-DIR";
  constexpr static uint64_t kBinDirValueHash = FNV(kBinDirValue);
  
  constexpr static std::string_view kScriptBinDirValue = "SCRIPT-BIN-DIR";
  constexpr static uint64_t kScriptBinDirValueHash = FNV(kScriptBinDirValue);

  constexpr static std::string_view kDebugValue = "DEBUG";
  constexpr static uint64_t kDebugValueHash = FNV(kDebugValue);

  constexpr static std::string_view kScenesValue = "SCENES";
  constexpr static uint64_t kScenesValueHash = FNV(kScenesValue);
  
  constexpr static std::string_view kAssetsValue = "ASSETS";
  constexpr static uint64_t kAssetsValueHash = FNV(kAssetsValue);

  constexpr static std::string_view kPrimarySceneValue = "PRIMARY-SCENE";
  constexpr static uint64_t kPrimarySceneHash = FNV(kPrimarySceneValue);
  
  constexpr static std::string_view kNeedPrimarySceneValue = "NEED-PRIMARY-SCENE";
  constexpr static uint64_t kNeedPrimarySceneHash = FNV(kNeedPrimarySceneValue);

  constexpr static std::string_view kHeightValue = "HEIGHT";
  constexpr static uint64_t kHeightValueHash = FNV(kHeightValue);

  constexpr static std::string_view kWidthValue = "WIDTH";
  constexpr static uint64_t kWidthValueHash = FNV(kWidthValue);

  constexpr static std::string_view kTitleValue = "TITLE";
  constexpr static uint64_t kTitleValueHash = FNV(kTitleValue);

  constexpr static std::string_view kXposValue = "XPOS";
  constexpr static uint64_t kXposValueHash = FNV(kXposValue);

  constexpr static std::string_view kYposValue = "YPOS";
  constexpr static uint64_t kYposValueHash = FNV(kYposValue);
  
  constexpr static std::string_view kCenteredValue = "CENTERED";
  constexpr static uint64_t kCenteredValueHash = FNV(kCenteredValue);

  constexpr static std::string_view kClearColorValue = "CLEAR-COLOR";
  constexpr static uint64_t kClearColorValueHash = FNV(kClearColorValue);

  constexpr static std::string_view kClearBuffersValue = "CLEAR-BUFFERS";
  constexpr static uint64_t kClearBuffersValueHash = FNV(kClearBuffersValue);
  
  constexpr static std::string_view kColorValue = "COLOR";
  constexpr static uint64_t kColorValueHash = FNV(kColorValue);

  constexpr static std::string_view kDepthValue = "DEPTH";
  constexpr static uint64_t kDepthValueHash = FNV(kDepthValue);

  constexpr static std::string_view kStencilValue = "STENCIL";
  constexpr static uint64_t kStencilValueHash = FNV(kStencilValue);
  
  constexpr static std::string_view kResizableValue = "RESIZABLE";
  constexpr static uint64_t kResizableValueHash = FNV(kResizableValue);

  constexpr static std::string_view kBorderlessValue = "BORDERLESS";
  constexpr static uint64_t kBorderlessValueHash = FNV(kBorderlessValue);

  constexpr static std::string_view kMaximizedValue = "MAXIMIZED";
  constexpr static uint64_t kMaximizedValueHash = FNV(kMaximizedValue);

  constexpr static std::string_view kMinimizedValue = "MINIMIZED";
  constexpr static uint64_t kMinimizedValueHash = FNV(kMinimizedValue);

  constexpr static std::string_view kViewportValue = "VIEWPORT";
  constexpr static uint64_t kViewportValueHash = FNV(kViewportValue);
  
  constexpr static std::string_view kClipValue = "CLIP";
  constexpr static uint64_t kClipValueHash = FNV(kClipValue);

  constexpr static std::string_view kAllowHighDpiValue = "ALLOW-HIGH-DPI";
  constexpr static uint64_t kAllowHighDpiValueHash = FNV(kAllowHighDpiValue);

  constexpr static std::string_view kMajorValue = "MAJOR";
  constexpr static uint64_t kMajorValueHash = FNV(kMajorValue);
  
  constexpr static std::string_view kMinorValue = "MINOR";
  constexpr static uint64_t kMinorValueHash = FNV(kMinorValue);

  constexpr static std::string_view kDoubleBufferValue = "DOUBLE-BUFFER";
  constexpr static uint64_t kDoubleBufferValueHash = FNV(kDoubleBufferValue);

  constexpr static std::string_view kDepthSizeValue = "DEPTH-SIZE";
  constexpr static uint64_t kDepthSizeValueHash = FNV(kDepthSizeValue);

  constexpr static std::string_view kStencilSizeValue = "STENCIL-SIZE";
  constexpr static uint64_t kStencilSizeValueHash = FNV(kStencilSizeValue);

  constexpr static std::string_view kAccelVisualValue = "ACCELERATED-VISUAL";
  constexpr static uint64_t kAccelVisualValueHash = FNV(kAccelVisualValue);

  constexpr static std::string_view kMultisampleBuffersValue = "MULTISAMPLE-BUFFERS";
  constexpr static uint64_t kMultisampleBuffersValueHash = FNV(kMultisampleBuffersValue);

  constexpr static std::string_view kMultisampleSamplesValue = "MULTISAMPLE-SAMPLES";
  constexpr static uint64_t kMultisampleSamplesValueHash = FNV(kMultisampleSamplesValue);

  constexpr static std::string_view kSrgbCapableValue = "SRGB-CAPABLE";
  constexpr static uint64_t kSrgbCapableValueHash = FNV(kSrgbCapableValue);

  constexpr static std::string_view kVsyncValue = "VSYNC";
  constexpr static uint64_t kVsyncValueHash = FNV(kVsyncValue);

  constexpr static std::string_view kDepthTestValue = "DEPTH-TEST";
  constexpr static uint64_t kDepthTestValueHash = FNV(kDepthTestValue);

  constexpr static std::string_view kStencilTestValue = "STENCIL-TEST";
  constexpr static uint64_t kStencilTestValueHash = FNV(kStencilTestValue);
  
  constexpr static std::string_view kCullFaceValue = "CULL-FACE";
  constexpr static uint64_t kCullFaceValueHash = FNV(kCullFaceValue);

  constexpr static std::string_view kMultisampleValue = "MULTISAMPLE";
  constexpr static uint64_t kMultisampleValueHash = FNV(kMultisampleValue);

  constexpr static std::string_view kBlendValue = "BLEND";
  constexpr static uint64_t kBlendValueHash = FNV(kBlendValue);

  constexpr static std::string_view kDisabledValue = "DISABLED";
  constexpr static uint64_t kDisabledValueHash = FNV(kDisabledValue);
  
  constexpr static std::string_view kMouseValue = "MOUSE";
  constexpr static uint64_t kMouseValueHash = FNV(kMouseValue);
  
  constexpr static std::string_view kLastMouseValue = "LAST-MOUSE";
  constexpr static uint64_t kLastMouseValueHash = FNV(kLastMouseValue);
  
  constexpr static std::string_view kDeltaMouseValue = "DELTA-MOUSE";
  constexpr static uint64_t kDeltaMouseValueHash = FNV(kDeltaMouseValue);

  constexpr static std::string_view kThemeValue = "THEME";
  constexpr static uint64_t kThemeValueHash = FNV(kThemeValue);

  constexpr static std::string_view kIniPathValue = "INI-PATH";
  constexpr static uint64_t kIniPathValueHash = FNV(kIniPathValue);

  constexpr static std::string_view kTypeValue = "TYPE";
  constexpr static uint64_t kTypeValueHash = FNV(kTypeValue);

  constexpr static std::string_view kDefaultModulesValue = "DEFAULT-MODULES";
  constexpr static uint64_t kDefaultModulesHash = FNV(kDefaultModulesValue);

  constexpr static std::string_view kModulesValue = "MODULES";
  constexpr static uint64_t kModulesValueHash = FNV(kModulesValue);
  
  constexpr static std::string_view kEntitiesValue = "ENTITIES";
  constexpr static uint64_t kEntitiesValueHash = FNV(kEntitiesValue);

  constexpr static std::string_view kPathValue = "PATH";
  constexpr static uint64_t kPathValueHash = FNV(kPathValue);
  
  constexpr static std::string_view kPathsValue = "PATHS";
  constexpr static uint64_t kPathsValueHash = FNV(kPathsValue);

  constexpr static std::string_view kUuidValue = "UUID";
  constexpr static uint64_t kUuidValueHash = FNV(kUuidValue);

  constexpr static std::string_view kComponentsValue = "COMPONENTS";
  constexpr static uint64_t kComponentsValueHash = FNV(kComponentsValue);
   
  constexpr static std::string_view kPositionValue = "POSITION";
  constexpr static uint64_t kPositionValueHash = FNV(kPositionValue);
  
  constexpr static std::string_view kDirectionValue = "DIRECTION";
  constexpr static uint64_t kDirectionValueHash = FNV(kDirectionValue);
  
  constexpr static std::string_view kUpValue = "UP";
  constexpr static uint64_t kUpValueHash = FNV(kUpValue);
  
  constexpr static std::string_view kWorldUpValue = "WORLD-UP";
  constexpr static uint64_t kWorldUpValueHash = FNV(kWorldUpValue);
  
  constexpr static std::string_view kRightValue = "RIGHT";
  constexpr static uint64_t kRightValueHash = FNV(kRightValue);
  
  constexpr static std::string_view kRotationValue = "ROTATION";
  constexpr static uint64_t kRotationValueHash = FNV(kRotationValue);
  
  constexpr static std::string_view kScaleValue = "SCALE";
  constexpr static uint64_t kScaleValueHash = FNV(kScaleValue);
  
  constexpr static std::string_view kChildrenValue = "CHILDREN";
  constexpr static uint64_t kChildrenValueHash = FNV(kChildrenValue);

  constexpr static std::string_view kScriptValue = "SCRIPT";
  constexpr static uint64_t kScriptValueHash = FNV(kScriptValue);
  
  constexpr static std::string_view kScriptsValue = "SCRIPTS";
  constexpr static uint64_t kScriptsValueHash = FNV(kScriptsValue);

  constexpr static std::string_view kCameraValue = "CAMERA";
  constexpr static uint64_t kCameraValueHash = FNV(kCameraValue);

  constexpr static std::string_view kPerspectiveCamValue = "PERSPECTIVE";
  constexpr static uint64_t kPerspectiveCamValueHash = FNV(kPerspectiveCamValue);
  
  constexpr static std::string_view kOrthographicCamValue = "ORTHOGRAPHIC";
  constexpr static uint64_t kOrthographicCamValueHash = FNV(kOrthographicCamValue);
  
  constexpr static std::string_view kSpeedValue = "SPEED";
  constexpr static uint64_t kSpeedValueHash = FNV(kSpeedValue);

  constexpr static std::string_view kSensitivityValue = "SENSITIVITY";
  constexpr static uint64_t kSensitivityValueHash = FNV(kSensitivityValue);

  constexpr static std::string_view kFovValue = "FOV";
  constexpr static uint64_t kFovValueHash = FNV(kFovValue);
  
  constexpr static std::string_view kZoomValue = "ZOOM";
  constexpr static uint64_t kZoomValueHash = FNV(kZoomValue);
  
  constexpr static std::string_view kConstrainPitchValue = "CONSTRAIN-PITCH";
  constexpr static uint64_t kConstrainPitchValueHash = FNV(kConstrainPitchValue);
  
  constexpr static std::string_view kPrimaryValue = "PRIMARY";
  constexpr static uint64_t kPrimaryValueHash = FNV(kPrimaryValue);

  constexpr static std::string_view kPhysicsValue = "PHYSICS";
  constexpr static uint64_t kPhysicsValueHash = FNV(kPhysicsValue);

  constexpr static std::string_view kRigidBody2DValue = "RIGID-BODY-2D";
  constexpr static uint64_t kRigidBody2DValueHash = FNV(kRigidBody2DValue);

  constexpr static std::string_view kStaticValue = "STATIC";
  constexpr static uint64_t kStaticValueHash = FNV(kStaticValue);

  constexpr static std::string_view kKinematicValue = "KINEMATIC";
  constexpr static uint64_t kKinematicValueHash = FNV(kKinematicValue);

  constexpr static std::string_view kDynamicValue = "DYNAMIC";
  constexpr static uint64_t kDynamicValueHash = FNV(kDynamicValue);

  constexpr static std::string_view kGravityValue = "GRAVITY";
  constexpr static uint64_t kGravityValueHash = FNV(kGravityValue);

  constexpr static std::string_view kMassValue = "MASS";
  constexpr static uint64_t kMassValueHash = FNV(kMassValue);

  constexpr static std::string_view kLinearDragValue = "LINEAR-DRAG";
  constexpr static uint64_t kLinearDragValueHash = FNV(kLinearDragValue);

  constexpr static std::string_view kAngularDragValue = "ANGULAR-DRAG";
  constexpr static uint64_t kAngularDragValueHash = FNV(kAngularDragValue);

  constexpr static std::string_view kGravityScaleValue = "GRAVITY-SCALE";
  constexpr static uint64_t kGravityScaleValueHash = FNV(kGravityScaleValue);

  constexpr static std::string_view kFixedRotationValue = "FIXED-ROTATION";
  constexpr static uint64_t kFixedRotationValueHash = FNV(kFixedRotationValue);

  constexpr static std::string_view kBulletValue = "BULLET";
  constexpr static uint64_t kBulletValueHash = FNV(kBulletValue);

  constexpr static std::string_view kCollider2DValue = "COLLIDER-2D";
  constexpr static uint64_t kCollider2DValueHash = FNV(kCollider2DValue);

  constexpr static std::string_view kOffsetValue = "OFFSET";
  constexpr static uint64_t kOffsetValueHash = FNV(kOffsetValue);
  
  constexpr static std::string_view kSizeValue = "SIZE";
  constexpr static uint64_t kSizeValueHash = FNV(kSizeValue);
  
  constexpr static std::string_view kDensityValue = "DENSITY";
  constexpr static uint64_t kDensityValueHash = FNV(kDensityValue);
  
  constexpr static std::string_view kFrictionValue = "FRICTION";
  constexpr static uint64_t kFrictionValueHash = FNV(kFrictionValue);

  constexpr static std::string_view kMeshValue = "MESH";
  constexpr static uint64_t kMeshValueHash = FNV(kMeshValue);
  
  constexpr static std::string_view kVisibleValue = "VISIBLE";
  constexpr static uint64_t kVisibleValueHash = FNV(kVisibleValue);
  
  constexpr static std::string_view kHandleValue = "HANDLE";
  constexpr static uint64_t kHandleValueHash = FNV(kHandleValue);
  
  constexpr static std::string_view kStaticMeshValue = "STATIC-MESH";
  constexpr static uint64_t kStaticMeshValueHash = FNV(kStaticMeshValue);
  
  constexpr static std::string_view kIsPrimitiveValue = "IS-PRIMITIVE";
  constexpr static uint64_t kIsPrimitiveValueHash = FNV(kIsPrimitiveValue);
  
  constexpr static std::string_view kPrimitiveValue = "PRIMITIVE";
  constexpr static uint64_t kPrimitiveValueHash = FNV(kPrimitiveValue);
    
  constexpr static std::string_view kRigidBodyValue = "RIGID-BODY";
  constexpr static uint64_t kRigidBodyValueHash = FNV(kRigidBodyValue);

  constexpr static std::string_view kDisableGravityValue = "DISABLE-GRAVITY";
  constexpr static uint64_t kDisableGravityValueHash = FNV(kDisableGravityValue);
  
  constexpr static std::string_view kIsTriggerValue = "IS-TRIGGER";
  constexpr static uint64_t kIsTriggerValueHash = FNV(kIsTriggerValue);
  
  constexpr static std::string_view kCollisionTypeValue = "COLLISION-TYPE";
  constexpr static uint64_t kCollisionTypeValueHash = FNV(kCollisionTypeValue);
  
  constexpr static std::string_view kDiscreteValue = "DISCRETE";
  constexpr static uint64_t kDiscreteValueHash = FNV(kDiscreteValue);
  
  constexpr static std::string_view kContinuousValue = "CONTINUOUS";
  constexpr static uint64_t kContinuousValueHash = FNV(kContinuousValue);
  
  constexpr static std::string_view kInitialLinearVelocityValue = "INITIAL-LINEAR-VELOCITY";
  constexpr static uint64_t kInitialLinearVelocityValueHash = FNV(kInitialLinearVelocityValue);
  
  constexpr static std::string_view kInitialAngularVelocityValue = "INITIAL-ANGULAR-VELOCITY";
  constexpr static uint64_t kInitialAngularVelocityValueHash = FNV(kInitialAngularVelocityValue);
  
  constexpr static std::string_view kMaxLinearVelocityValue = "INITIAL-LINEAR-VELOCITY";
  constexpr static uint64_t kMaxLinearVelocityValueHash = FNV(kMaxLinearVelocityValue);
  
  constexpr static std::string_view kMaxAngularVelocityValue = "INITIAL-ANGULAR-VELOCITY";
  constexpr static uint64_t kMaxAngularVelocityValueHash = FNV(kMaxAngularVelocityValue);
  
  constexpr static std::string_view kColliderValue = "COLLIDER";
  constexpr static uint64_t kColliderValueHash = FNV(kColliderValue);

  constexpr static std::string_view kLightSourceValue = "LIGHT-SOURCE";
  constexpr static uint64_t kLightSourceHash = FNV(kLightSourceValue);

} // namespace other 

#endif // !OTHER_ENGINE_CONFIG_KEYS_HPP
