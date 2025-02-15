using System.Text;
using Other;

namespace Forest {

  public sealed class CharacterController : OtherObject {
    Camera cam = null;
    Transform transform = null;
    RigidBody rigid_body = null;

    Vec3 velocity = Vec3.zero;

    // public string axis_look_horzizontal = "Mouse X";             // Mouse to Look
    // public string axis_look_vertical    = "Mouse Y";             // 
    // public string axis_move_horzizontal = "Horizontal";          // WASD to Move
    // public string axis_move_vertical    = "Vertical";            // 
    // public KeyCode key_run = KeyCode.KEY_LSHIFT;     // Left Shift to Run
    // public KeyCode key_crouch = KeyCode.KEY_LCTRL;   // Left Control to Crouch
    // public KeyCode key_jump = KeyCode.KEY_SPACE;         // Space to Jump
    // public KeyCode keySlide = KeyCode.KEY_F;             // F to Slide (only when running)
    // public KeyCode keyToggleCursor = KeyCode.KEY_BACKQ;     // ` to toggle lock cursor (aka [~] console key)
    
    // Input Variables that can be assigned externally
    // the cursor can also be manually locked or freed by calling the public void SetLockCursor( bool doLock )
    public float input_look_x = 0;
    public float input_look_y = 0;

    public bool forward_key = false;  // Held or Pressed
    public bool backward_key = false;  // Held or Pressed

    public bool left_key = false;  // Held or Pressed
    public bool right_key = false;  // Held or Pressed

    public bool run_key = false;  // Held
    public bool crouch_key = false;  // Held
    public bool jump_key = false;  // Pressed
    public bool cursor_key = false;  // Pressed
    public bool is_grounded = true;
    public bool is_ceiling = false;
   
    // [Header("Look Settings")]
    // public float mouseSensitivityX = 2f;  // speed factor of look X
    // public float mouseSensitivityY = 2f;             // speed factor of look Y
    // [Tooltip("larger values for less filtering, more responsiveness")]
    // public float mouseSnappiness = 20f;              // default was 10f; larger values of this cause less filtering, more responsiveness
    // public bool invertLookY = false;                 // toggle invert look Y
    // public float clampLookY = 90f;                   // maximum look up/down angle

    // [Header("Grounded Settings")]
    // [Tooltip("The starting position of the isGrounded spherecast. Set to the sphereCastRadius plus the CC Skin Width. Enable showGizmos to visualize.")]
    // this should be just above the base of the cc, in the amount of the skin width (in case the cc sinks in)
    public float start_dist_from_bottom = 0.2f; 
    public float ground_check_y = 0.33f;               // 0.25 + 0.08 (sphereCastRadius + CC skin width)
    // [Tooltip("The position of the ceiling checksphere. Set to the height minus sphereCastRadius plus the CC Skin Width. Enable showGizmos to visualize.")]
    // this should extend above the cc (by approx skin width) so player can still move when not at full height (not crouching, trying to stand up), 
    // otherwise if it's below the top then the cc gets stuck
    public float ceiling_check_y = 1.83f;              // 2.00 - 0.25 + 0.08 (height - sphereCastRadius + CC skin width) 
    // [Space(5)]
    public float sphere_cast_radius = 0.25f;           // radius of area to detect for ground
    public float sphere_cast_dist = 0.75f;         // How far spherecast moves down from origin point
    // [Space(5)]
    public float raycast_length = 75f;// 0.75f;              // secondary raycasts (match to sphereCastDistance)
    // public Vec3 ray_origin_offset1 = new Vec3(-0.2f, 0f, 0.16f);
    // public Vec3 ray_origin_offset2 = new Vec3(0.2f, 0f, -0.16f);
   
    // [Header("Move Settings")]
    public float crouch_speed = 0.1f;                   // crouching movement speed
    public float walk_speed = 0.25f;                     // regular movement speed
    public float run_speed = 0.4f;                     // run movement speed
    public float gravity = -9.81f;                   // gravity / fall rate
    public float jump_height = 2.5f;                  // jump height

    private float default_height = 0;                 // reference to scale player crouch
    private float camera_start_y = 0;                  // reference to move camera with crouch

    // [Header("- reference variables -")]
    public float xrot = 0f;                     // the up/down angle the player is looking
    
    
    private float acc_mouse_x = 0;                     // reference for mouse look smoothing
    private float acc_mouse_y = 0;                     // reference for mouse look smoothing
    public float look_sensitivity = 1f;               // default was 2f; speed factor of look X and Y
    public float mouse_snappiness = 10f;              // default was 10f; larger values of this cause less filtering, more responsiveness
    public bool constrain_pitch = true;               // toggle to constrain pitch

    public float ground_slope_angle = 0f;              // Angle of the slope in degrees
    private float ground_offset_y = 0;                 // calculated offset relative to height
    private float ceiling_offset_y = 0;                // calculated offset relative to height

    public Vec3 ground_slope_dir = Vec3.zero;    // The calculated slope as a vector
    
    CharacterMovement character_movement = null;

    private void Initialize() {
      rigid_body = GetComponent<RigidBody>();
      transform = GetComponent<Transform>();

      cam = GetComponent<Camera>();
      camera_start_y = cam.Position.y;
      default_height = camera_start_y;

      Vec3 real_cam_pos = new Vec3(transform.Position.x, camera_start_y, transform.Position.z);
      cam.Position = real_cam_pos;

      StringBuilder sb = new StringBuilder();
      sb.Append("Character Controller Initialized :\n")
        .Append($"  > Default Height : {default_height}\n")
        .Append($"  > Camera Start Y : {camera_start_y}\n")
        .Append($"  > Ground Check Y : {ground_check_y}\n")
        .Append($"  > Ceiling Check Y : {ceiling_check_y}\n")
        .Append($"  > Sphere Cast Radius : {sphere_cast_radius}\n")
        .Append($"  > Sphere Cast Distance : {sphere_cast_dist}\n")
        .Append($"  > Raycast Length : {raycast_length}\n");

      /// Initial physics raycast to get ground and ceiling offsets
      PhysicsRaycastHit floor_hit;
      PhysicsRaycastHit ceiling_hit;

      /// because transform position is at center of object (and therefore inside the collider),
      ///   we need to raycast from the outside of the player, this approach only wworks for capsule colliders
      // Vec3 modified_pos = transform.Position;
      // float radius = transform.Scale.x / 2f;
      // /// move the way we are facing by the radius of the player with a small offset
      // modified_pos += cam.Forward.Normalized() * (radius + 0.2f);
      /*
        // FIRST RAYCAST
        if (Physics.Raycast(origin + rayOriginOffset1, Vector3.down, out slopeHit1, raycastLength)) {
            // Debug line to first hit point
            #if UNITY_EDITOR
            if (showGizmos) { Debug.DrawLine(origin + rayOriginOffset1, slopeHit1.point, Color.red); }
            #endif
            // Get angle of slope on hit normal
            float angleOne = Vector3.Angle(slopeHit1.normal, Vector3.up);

          // 2ND RAYCAST
          if (Physics.Raycast(origin + rayOriginOffset2, Vector3.down, out slopeHit2, raycastLength)) {
              // Debug line to second hit point
              #if UNITY_EDITOR
              if (showGizmos) { Debug.DrawLine(origin + rayOriginOffset2, slopeHit2.point, Color.red); }
              #endif
              // Get angle of slope of these two hit points.
              float angleTwo = Vector3.Angle(slopeHit2.normal, Vector3.up);
              // 3 collision points: Take the MEDIAN by sorting array and grabbing middle.
              float[] tempArray = new float[] { groundSlopeAngle, angleOne, angleTwo };
              System.Array.Sort(tempArray);
              groundSlopeAngle = tempArray[1];
          } else {
              // 2 collision points (sphere and first raycast): AVERAGE the two
              float average = (groundSlopeAngle + angleOne) / 2;
              groundSlopeAngle = average;
          }
      }
      */
      Vec3 modified_pos = new Vec3(0f, 10f, 0f);
      Logger.WriteDebug($"Modified Position : {modified_pos} \\ ray length = {raycast_length}");

      if (Physics.Raycast(modified_pos, Vec3.down, raycast_length, out floor_hit)) {
        Logger.WriteDebug("Floor Hit");
        ground_offset_y = floor_hit.distance;
      }
      if (Physics.Raycast(modified_pos, Vec3.up, raycast_length, out ceiling_hit)) {
        Logger.WriteDebug("Ceiling Hit");
        ceiling_offset_y = ceiling_hit.distance;
      }
       
      character_movement = new CharacterMovement(cam, Vec3.down * gravity, cam.Forward, cam.Right, transform, (e, s, ns) => {
        Logger.WriteDebug($"State Change : {e} : {s} -> {ns}");
      });
      character_movement.Speed = run_speed;

      sb.Append($"  > Ground Offset Y : {ground_offset_y}\n");
      sb.Append($"  > Ceiling Offset Y : {ceiling_offset_y}\n");

      RefreshCursor();

      Logger.WriteDebug(sb.ToString());
    }

    public override void OnStart() {
      Initialize();
    }

    public override void Update(float dt) {
      ProcessInputs();
      ProcessMovement(dt);
    }

    protected override void BeginContact(OtherObject other) {}

    protected override void HandleContact(OtherObject other, Vec3 point) {}

    protected override void EndContact(OtherObject other) {}

    // lock/hide or show/unlock cursor
    private void SetLockCursor(bool do_lock) {
      // cursorActive = doLock;
      // RefreshCursor();
    }

    private void ToggleLockCursor() {
      // cursorActive = !cursorActive;
      // RefreshCursor();
    }

    private void RefreshCursor() {
      // if ( !cursorActive && Cursor.lockState != CursorLockMode.Locked )   { Cursor.lockState = CursorLockMode.Locked; }
      // if (  cursorActive && Cursor.lockState != CursorLockMode.None   )   { Cursor.lockState = CursorLockMode.None;   }
    }

    private void ProcessInputs() {
      forward_key = Keyboard.KeyHeld(KeyCode.KEY_W) || Keyboard.KeyPressed(KeyCode.KEY_W);
      backward_key = Keyboard.KeyHeld(KeyCode.KEY_S) || Keyboard.KeyPressed(KeyCode.KEY_S);

      left_key = Keyboard.KeyHeld(KeyCode.KEY_A) || Keyboard.KeyPressed(KeyCode.KEY_A);
      right_key = Keyboard.KeyHeld(KeyCode.KEY_D) || Keyboard.KeyPressed(KeyCode.KEY_D);

      if (forward_key && backward_key) {
        forward_key = false;
        backward_key = false;
      }

      if (left_key && right_key) {
        left_key = false;
        right_key = false;
      }

      run_key = Keyboard.KeyHeld(KeyCode.KEY_LSHIFT) || Keyboard.KeyPressed(KeyCode.KEY_LSHIFT);
      crouch_key = Keyboard.KeyHeld(KeyCode.KEY_LCTRL) || Keyboard.KeyPressed(KeyCode.KEY_LCTRL);

      jump_key = Keyboard.KeyPressed(KeyCode.KEY_SPACE) || Keyboard.KeyHeld(KeyCode.KEY_SPACE);
    }

    private MovementDirection GetMovementUpdate() {
      MovementDirection move = MovementDirection.None;
      if (forward_key) {
        move |= MovementDirection.Forward;
      } else if (backward_key) {
        move |= MovementDirection.Backward;
      }

      if (right_key) {
        move |= MovementDirection.Right;
      } else if (left_key) {
        move |= MovementDirection.Left;
      } 

      return move;
    }

    private void ProcessMovement(float dt) {
      /// look first so that camera/orientation is correct before physics calculations
      character_movement.Look(dt);

      // GroundCheck();
      // CeilingCheck();

      MovementDirection dir = GetMovementUpdate();
      var (new_pos, new_velocity) = character_movement.Move(dt, dir, jump_key, crouch_key);
      /// update rigid body cause character_movement only updates player transform but the 
      //    collider/physics body needs to move as well
      rigid_body.Position = new_pos;

      velocity = new_velocity;

      // crouch/stand up smoothly
      // float last_height = curr_position.y; 
      // float next_height = 2f; // Mathf.Lerp(controller.height, h, 5f * Time.deltaTime);

      // if crouching, or only stand if there is no ceiling
      // if (next_height < last_height || !is_ceiling) {
      //   // controller.height = Mathf.Lerp( controller.height, h, 5f * Time.deltaTime );

      //   // fix vertical position
      //   calc = curr_position;
      //   // calc.y += (controller.height - last_height) / 2f;
      //   transform.Position = calc;

      //   // offset camera
      //   calc = cam.Position;
      //   // calc.y = ( controller.height / defaultHeight ) + cameraStartY - ( defaultHeight * 0.5f );
      //   cam.Position = calc;

      //   // calculate offset
      //   float height_factor = 1f; // (default_height - controller.height ) * 0.5f;
        
      //   // offset ground check
      //   ground_offset_y = height_factor + ground_check_y;
        
      //   // offset ceiling check
      //   // ceiling_offset_y = height_factor + controller.height - (default_height - ceiling_check_y);
      // } 

       
      // if (is_grounded) {
      //   faux_gravity.x = 0;
      //   faux_gravity.z = 0;

      //   // constant grounded gravity
      //   if (faux_gravity.y < 0) {
      //     //faux_gravity.y = -1f;
      //     // faux_gravity.y = Mathf.Lerp( faux_gravity.y, -1f, 4f * Time.deltaTime );
      //   }

      //   if (!is_ceiling && jump_key) {
      //     // faux_gravity.y = Mathf.Sqrt( jumpHeight * -2f * gravity );
      //   }

      //   float lerpFactor = (last_speed > next_speed ? 4f : 2f);
      //   // speed = Mathf.Lerp(last_speed, next_speed, lerpFactor * dt);
      // } else {
      //   // speed = Mathf.Lerp(last_speed, next_speed, 0.125f * dt);
      // }

      // prevent floating if jumping into a ceiling
      // if (is_ceiling) {
      //   speed = crouch_speed;

      //   if (faux_gravity.y > 0) {
      //     faux_gravity.y = -1f; // 0;
      //   }
      // }
      // faux_gravity.y += gravity * dt;

      // controller.Move(calc);
      
      // #if UNITY_EDITOR
      // // slope angle and fauxGravity debug info
      // if ( showGizmos ) 
      // { 
      //     calc = playerTx.position;
      //     calc.y += groundOffsetY;
      //     Debug.DrawRay( calc, groundSlopeDir.normalized * 5f, Color.blue ); 
      //     Debug.DrawRay( calc, fauxGravity, Color.green ); 
      // }
      // #endif
    }

    private void CeilingCheck() {
      // Vec3 position = transform.Position;
      // Vec3 origin = new Vec3(position.x, position.y + ceiling_offset_y, position.z);
      // is_ceiling = Physics.CheckSphere( origin, sphereCastRadius, castingMask );
    }
   
    // find if isGrounded, slope angle and directional vector
    void GroundCheck() {
      // //Vector3 origin = new Vector3( transform.position.x, transform.position.y - (controller.height / 2) + startDistanceFromBottom, transform.position.z );
      // Vector3 origin = new Vector3( playerTx.position.x, playerTx.position.y + groundOffsetY, playerTx.position.z );

      // // Out hit point from our cast(s)
      // RaycastHit hit;

      // // SPHERECAST
      // // "Casts a sphere along a ray and returns detailed information on what was hit."
      // if (Physics.SphereCast(origin, sphereCastRadius, Vector3.down, out hit, sphereCastDistance, castingMask)) {
    //     // Angle of our slope (between these two vectors). 
    //     // A hit normal is at a 90 degree angle from the surface that is collided with (at the point of collision).
    //     // e.g. On a flat surface, both vectors are facing straight up, so the angle is 0.
    //     groundSlopeAngle = Vector3.Angle(hit.normal, Vector3.up);

    //     // Find the vector that represents our slope as well. 
    //     //  temp: basically, finds vector moving across hit surface 
    //     Vector3 temp = Vector3.Cross(hit.normal, Vector3.down);
    //     //  Now use this vector and the hit normal, to find the other vector moving up and down the hit surface
    //     groundSlopeDir = Vector3.Cross(temp, hit.normal); 

      //     // --
      //     isGrounded = true;
      // } else {
      //     isGrounded = false;
      // }   // --

      // // Now that's all fine and dandy, but on edges, corners, etc, we get angle values that we don't want.
      // // To correct for this, let's do some raycasts. You could do more raycasts, and check for more
      // // edge cases here. There are lots of situations that could pop up, so test and see what gives you trouble.
      // RaycastHit slopeHit1;
      // RaycastHit slopeHit2;

      // // FIRST RAYCAST
      // if (Physics.Raycast(origin + rayOriginOffset1, Vector3.down, out slopeHit1, raycastLength)) {
      //     // Debug line to first hit point
      //     #if UNITY_EDITOR
      //     if (showGizmos) { Debug.DrawLine(origin + rayOriginOffset1, slopeHit1.point, Color.red); }
      //     #endif
      //     // Get angle of slope on hit normal
      //     float angleOne = Vector3.Angle(slopeHit1.normal, Vector3.up);

      //     // 2ND RAYCAST
      //     if (Physics.Raycast(origin + rayOriginOffset2, Vector3.down, out slopeHit2, raycastLength)) {
      //         // Debug line to second hit point
      //         #if UNITY_EDITOR
      //         if (showGizmos) { Debug.DrawLine(origin + rayOriginOffset2, slopeHit2.point, Color.red); }
      //         #endif
      //         // Get angle of slope of these two hit points.
      //         float angleTwo = Vector3.Angle(slopeHit2.normal, Vector3.up);
      //         // 3 collision points: Take the MEDIAN by sorting array and grabbing middle.
      //         float[] tempArray = new float[] { groundSlopeAngle, angleOne, angleTwo };
      //         System.Array.Sort(tempArray);
      //         groundSlopeAngle = tempArray[1];
      //     } else {
      //         // 2 collision points (sphere and first raycast): AVERAGE the two
      //         float average = (groundSlopeAngle + angleOne) / 2;
      //         groundSlopeAngle = average;
      //     }
      // }
    }
  }

  
  // public struct CharacterMovementDirection {
  //   private float speed = 0;
  //   private Vec3 movement_dir;

  //   public bool forward_key = false;  // Held or Pressed
  //   public bool backward_key = false;  // Held or Pressed

  //   public bool left_key = false;  // Held or Pressed
  //   public bool right_key = false;  // Held or Pressed

  //   public bool run_key = false;  // Held
  //   public bool crouch_key = false;  // Held
  //   public bool jump_key = false;  // Pressed
  //   public bool slide_key = false;  // Pressed
  //   public bool cursor_key = false;  // Pressed

  //   CharacterMovementDirection(float speed) {
  //     this.speed = speed;
  //     this.movement_dir = Vec3.zero;
  //   }

  //   public void SetSpeed(float speed) {
  //     this.speed = speed;
  //   }

  //   public Vec3 GetMovementDirection() {
  //     return movement_dir;
  //   }
  // }

}

/*
// ------------------------------------------ 
// BasicFPCC.cs
// a basic first person character controller
// with jump, crouch, run, slide
// 2020-10-04 Alucard Jay Kay 
// ------------------------------------------ 

// source : 
// https://discussions.unity.com/t/855344
// Brackeys FPS controller base : 
// https://www.youtube.com/watch?v=_QajrabyTJc
// smooth mouse look : 
// https://discussions.unity.com/t/710168/2
// ground check : (added isGrounded)
// https://gist.github.com/jawinn/f466b237c0cdc5f92d96
// run, crouch, slide : (added check for headroom before un-crouching)
// https://answers.unity.com/questions/374157/character-controller-slide-action-script.html
// interact with rigidbodies : 
// https://docs.unity3d.com/2018.4/Documentation/ScriptReference/CharacterController.OnControllerColliderHit.html

// ** SETUP **
// Assign the BasicFPCC object to its own Layer
// Assign the Layer Mask to ignore the BasicFPCC object Layer
// CharacterController (component) : Center => X 0, Y 1, Z 0
// Main Camera (as child) : Transform : Position => X 0, Y 1.7, Z 0
// (optional GFX) Capsule primitive without collider (as child) : Transform : Position => X 0, Y 1, Z 0
// alternatively : 
// at the end of this script is a Menu Item function to create and auto-configure a BasicFPCC object
// GameObject -> 3D Object -> BasicFPCC

using System.Collections;
using System.Collections.Generic;
using UnityEngine;

#if UNITY_EDITOR // only required if using the Menu Item function at the end of this script
using UnityEditor; 
#endif

[RequireComponent(typeof(CharacterController))]
public class BasicFPCC : MonoBehaviour
{
    void ProcessMovement()
    {
        // - variables -
        float vScale = 1f; // for calculating GFX scale (optional)
        float h = defaultHeight;
        float nextSpeed = walkSpeed;
        Vector3 calc; // used for calculations
        Vector3 move; // direction calculation
       
        // player current speed
        float currSpeed = ( playerTx.position - lastPos ).magnitude / Time.deltaTime;
        currSpeed = ( currSpeed < 0 ? 0 - currSpeed : currSpeed ); // abs value

        // - Check if Grounded -
        GroundCheck();

        isSlipping = ( groundSlopeAngle > controller.slopeLimit ? true : false );

        // - Check Ceiling above for Head Room -
        CeilingCheck();

        // - Run and Crouch -

        // if grounded, and not stuck on ceiling
        if ( isGrounded && !isCeiling && inputKeyRun )
        {
            nextSpeed = runSpeed; // to run speed
        }

        if ( inputKeyCrouch ) // crouch
        {
            vScale = 0.5f;
            h = 0.5f * defaultHeight;
            nextSpeed = crouchSpeed; // slow down when crouching
        }   

        // - Slide -

        // if not sliding, and not stuck on ceiling, and is running
        if ( !isSliding && !isCeiling && inputKeyRun && inputKeyDownSlide ) // slide
        {
            // check velocity is faster than walkSpeed
            if ( currSpeed > walkSpeed )
            {
                slideTimer = 0; // start slide timer
                isSliding = true;
                slideForward = ( playerTx.position - lastPos ).normalized;
            }
        }
        lastPos = playerTx.position; // update reference

        // check slider timer and velocity
        if ( isSliding )
        {
            nextSpeed = currSpeed; // default to current speed
            move = slideForward; // set input to direction of slide

            slideTimer += Time.deltaTime; // slide timer
           
            // if timer max, or isSliding and not moving, then stop sliding
            if ( slideTimer > slideDuration || currSpeed < crouchSpeed )
            {
                isSliding = false;
            }
            else // confirmed player is sliding
            {
                vScale = 0.5f;            // gfx scale
                h = 0.5f * defaultHeight; // height is crouch height
                nextSpeed = slideSpeed;   // to slide speed
            }
        }
        else // - Player Move Input -
        {
            move = ( playerTx.right * inputMoveX ) + ( playerTx.forward * inputMoveY );

            if ( move.magnitude > 1f )
            {
                move = move.normalized;
            }
        }

        // - Height -

        // crouch/stand up smoothly
        float lastHeight = controller.height; 
        float nextHeight = Mathf.Lerp( controller.height, h, 5f * Time.deltaTime );

        // if crouching, or only stand if there is no ceiling
        if ( nextHeight < lastHeight || !isCeiling )
        {
            controller.height = Mathf.Lerp( controller.height, h, 5f * Time.deltaTime );

            // fix vertical position
            calc = playerTx.position;
            calc.y += ( controller.height - lastHeight ) / 2f;
            playerTx.position = calc;

            // offset camera
            calc = cameraTx.localPosition;
            calc.y = ( controller.height / defaultHeight ) + cameraStartY - ( defaultHeight * 0.5f );
            cameraTx.localPosition = calc;

            // calculate offset
            float heightFactor = ( defaultHeight - controller.height ) * 0.5f;
           
            // offset ground check
            groundOffsetY = heightFactor + groundCheckY;
           
            // offset ceiling check
            ceilingOffsetY = heightFactor + controller.height - ( defaultHeight - ceilingCheckY );

            // scale gfx (optional)
            if ( playerGFX )
            {
                calc = playerGFX.localScale;
                calc.y = Mathf.Lerp( calc.y, vScale, 5f * Time.deltaTime );
                playerGFX.localScale = calc;
            }
        }

        // - Slipping Jumping Gravity - 

        // smooth speed
        float speed;
       
        if ( isGrounded )
        {
            if ( isSlipping ) // slip down slope
            {
                // movement left/right while slipping down
                // player rotation to slope
                Vector3 slopeRight = Quaternion.LookRotation( Vector3.right ) * groundSlopeDir;
                float dot = Vector3.Dot( slopeRight, playerTx.right );
                // move on X axis, with Y rotation relative to slopeDir
                move = slopeRight * ( dot > 0 ? inputMoveX : -inputMoveX );

                // speed
                nextSpeed = Mathf.Lerp( currSpeed, runSpeed, 5f * Time.deltaTime );

                // increase angular gravity
                float mag = fauxGravity.magnitude;
                calc = Vector3.Slerp( fauxGravity, groundSlopeDir * runSpeed, 4f * Time.deltaTime );
                fauxGravity = calc.normalized * mag;
            }
            else
            {
                // reset angular fauxGravity movement
                fauxGravity.x = 0;
                fauxGravity.z = 0;

                if ( fauxGravity.y < 0 ) // constant grounded gravity
                {
                    //fauxGravity.y = -1f;
                    fauxGravity.y = Mathf.Lerp( fauxGravity.y, -1f, 4f * Time.deltaTime );
                }
            }

            // - Jump -
            if ( !isSliding && !isCeiling && inputKeyDownJump ) // jump
            {
                fauxGravity.y = Mathf.Sqrt( jumpHeight * -2f * gravity );
            }

            // --

            // - smooth speed -
            // take less time to slow down, more time speed up
            float lerpFactor = ( lastSpeed > nextSpeed ? 4f : 2f );
            speed = Mathf.Lerp( lastSpeed, nextSpeed, lerpFactor * Time.deltaTime );
        }
        else // no friction, speed changes slower
        {
            speed = Mathf.Lerp( lastSpeed, nextSpeed, 0.125f * Time.deltaTime );
        }

        // prevent floating if jumping into a ceiling
        if ( isCeiling )
        {
            speed = crouchSpeed; // clamp speed to crouched

            if ( fauxGravity.y > 0 )
            {
                fauxGravity.y = -1f; // 0;
            }
        }

        lastSpeed = speed; // update reference

        // - Add Gravity -

        fauxGravity.y += gravity * Time.deltaTime;
       
        // - Move -

        calc = move * speed * Time.deltaTime;
        calc += fauxGravity * Time.deltaTime;

        controller.Move( calc );

        // - DEBUG - 
       
        #if UNITY_EDITOR
        // slope angle and fauxGravity debug info
        if ( showGizmos ) 
        { 
            calc = playerTx.position;
            calc.y += groundOffsetY;
            Debug.DrawRay( calc, groundSlopeDir.normalized * 5f, Color.blue ); 
            Debug.DrawRay( calc, fauxGravity, Color.green ); 
        }
        #endif
    }
   
    // this script pushes all rigidbodies that the character touches
    void OnControllerColliderHit( ControllerColliderHit hit )
    {
        Rigidbody body = hit.collider.attachedRigidbody;

        // no rigidbody
        if ( body == null || body.isKinematic )
        {
            return;
        }

        // We dont want to push objects below us
        if ( hit.moveDirection.y < -0.3f )
        {
            return;
        }

        // If you know how fast your character is trying to move,
        // then you can also multiply the push velocity by that.
        body.velocity = hit.moveDirection * lastSpeed;
    }

    // Debug Gizmos
    #if UNITY_EDITOR
    void OnDrawGizmosSelected()
    {
        if ( showGizmos )
        {
            if ( !Application.isPlaying )
            {
                groundOffsetY = groundCheckY;
                ceilingOffsetY = ceilingCheckY;
            }

            Vector3 startPoint = new Vector3( transform.position.x, transform.position.y + groundOffsetY, transform.position.z );
            Vector3 endPoint = startPoint + new Vector3( 0, -sphereCastDistance, 0 );
            Vector3 ceilingPoint = new Vector3( transform.position.x, transform.position.y + ceilingOffsetY, transform.position.z );

            Gizmos.color = ( isGrounded == true ? Color.green : Color.white );
            Gizmos.DrawWireSphere( startPoint, sphereCastRadius );

            Gizmos.color = Color.gray;
            Gizmos.DrawWireSphere( endPoint, sphereCastRadius );

            Gizmos.DrawLine( startPoint, endPoint );

            Gizmos.color = ( isCeiling == true ? Color.red : Color.white );
            Gizmos.DrawWireSphere( ceilingPoint, sphereCastRadius );
        }
    }
    #endif
}


// =======================================================================================================================================

// ** DELETE from here down, if menu item and auto configuration is NOT Required **

// this section adds create BasicFPCC object to the menu : New -> GameObject -> 3D Object
// then configures the gameobject
// demo layer used : Ignore Raycast
// also finds the main camera, attaches and sets position
// and creates capsule gfx object (for visual while editing)

//#if UNITY_EDITOR
//using UnityEditor;
//#endif

public class BasicFPCC_Setup : MonoBehaviour
{
    #if UNITY_EDITOR

    private static int playerLayer = 2; // default to the Ignore Raycast Layer (to demonstrate configuration)

    [MenuItem("GameObject/3D Object/BasicFPCC", false, 0)]
    public static void CreateBasicFPCC() 
    {
        GameObject go = new GameObject( "Player" );

        CharacterController controller = go.AddComponent< CharacterController >();
        controller.center = new Vector3( 0, 1, 0 );

        BasicFPCC basicFPCC = go.AddComponent< BasicFPCC >();

        // Layer Mask
        go.layer = playerLayer;
        basicFPCC.castingMask = ~(1 << playerLayer);
        Debug.LogError( "** SET the LAYER of the PLAYER Object, and the LAYERMASK of the BasicFPCC castingMask **" );
        Debug.LogWarning( 
            "Assign the BasicFPCC Player object to its own Layer, then assign the Layer Mask to ignore the BasicFPCC Player object Layer. Currently using layer " 
            + playerLayer.ToString() + ": " + LayerMask.LayerToName( playerLayer ) 
        );

        // Main Camera
        GameObject mainCamObject = GameObject.Find( "Main Camera" );
        if ( mainCamObject )
        {
            mainCamObject.transform.parent = go.transform;
            mainCamObject.transform.localPosition = new Vector3( 0, 1.7f, 0 );
            mainCamObject.transform.localRotation = Quaternion.identity;

            basicFPCC.cameraTx = mainCamObject.transform;
        }
        else // create example camera
        {
            Debug.LogError( "** Main Camera NOT FOUND ** \nA new Camera has been created and assigned. Please replace this with the Main Camera (and associated AudioListener)." );

            GameObject camGo = new GameObject( "BasicFPCC Camera" );
            camGo.AddComponent< Camera >();
           
            camGo.transform.parent = go.transform;
            camGo.transform.localPosition = new Vector3( 0, 1.7f, 0 );
            camGo.transform.localRotation = Quaternion.identity;

            basicFPCC.cameraTx = camGo.transform;
        }

        // GFX
        GameObject gfx = GameObject.CreatePrimitive( PrimitiveType.Capsule );
        Collider cc = gfx.GetComponent< Collider >();
        DestroyImmediate( cc );
        gfx.transform.parent = go.transform;
        gfx.transform.localPosition = new Vector3( 0, 1, 0 );
        gfx.name = "GFX";
        gfx.layer = playerLayer;
        basicFPCC.playerGFX = gfx.transform;
    }
    #endif
}
*/