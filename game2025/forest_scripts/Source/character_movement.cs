using System;
using System.Collections.Generic;
using Other;

namespace Forest {


  [Flags]
  public enum MovementState {
    Idle = 0,
    Walking = 1 << 0,
    Running = 1 << 1,
    Jumping = 1 << 2,
    Falling = 1 << 3,
    Crouching = 1 << 4,
  }

  [Flags]
  public enum MovementDirection {
    None = 0,
    Forward = 1 << 0,
    Backward = 1 << 1,
    Left = 1 << 2,
    Right = 1 << 3,
  }

  public enum MovementEvent {
    JumpBegin,
    JumpApex,
    BeginFalling,
    JumpLand,

    CrouchBegin,
    CrouchEnd,
    StandFromCrouchBegin,
    StandFromCrouchEnd,

    BeginWalking,
    StopWalking,

    BeginRunningFromIdle,
    BeginRunningFromWalking,
    StopRunning
  }

  public delegate void StateChangeHandler(MovementEvent evnt, MovementState old_state, MovementState new_state);

  class CharacterMovement {
    Transform transform;
    Camera player_viewpoint;
    Interval pitch_interval;

    private Vec3 gravity;
    private Vec3 linear_velocity;

    private Vec3 acceleration;

    private Vec3 forward;
    private Vec3 right;

    private float current_speed;
    private float walk_speed;
    private float run_speed;
    private float crouch_speed;
    private float max_speed;
    
    private float max_acceleration;

    private float jump_speed;

    private float current_jump_height;
    private float max_jump_height;
    
    private Dictionary<(MovementState, MovementEvent), MovementState> state_transitions;
    private MovementState state;

#nullable enable
    private StateChangeHandler? on_state_change = null;

    public CharacterMovement(Camera player_viewpoint, Vec3 gravity, Vec3 forward, Vec3 right, Transform transform, StateChangeHandler? on_state_change = null) {
      this.player_viewpoint = player_viewpoint;
      /// we do custom pitch constraints
      this.player_viewpoint.PitchConstrained = false;
      pitch_interval = new Interval(-28f, 89f);

      this.on_state_change = on_state_change;
      this.transform = transform;
      this.forward = forward;
      this.right = right;
      Initialize(gravity);
    }
#nullable disable

    public (Vec3,Vec3) Move(float dt, MovementDirection direction, bool jump_key, bool crouch_key) {
      if (!InControllableState()) {
        /// if we are not in idle or walking state then we are in a state where we are not moving or should not be moving directly
        ///   i.e falling, jumping, etc... these should be implicitly updated as time progresses until we land, reach apex, etc...
        return (transform.Position, LinearVelocity);
      }

      /// update velocity before transition so that we can use the update velocity to determine if we should transition
      UpdateVelocity(dt, direction, jump_key, crouch_key);
      switch (state) {
        case MovementState.Idle:
          if (linear_velocity.Magnitude() > 0.0f) {
            Transition(MovementEvent.BeginWalking);
          }
          break;
        case MovementState.Running:
          /// TODO: consider adding extra logic to slow from running to walking if velocity is below a certain threshold
        case MovementState.Crouching:
        case MovementState.Walking:
          if (linear_velocity.Magnitude() == 0.0f) {
            Transition(MovementEvent.StopWalking);
          }
          break;
      }


      Vec3 calculated_velocity = linear_velocity;
      LinearVelocity = Vec3.zero;

      MoveViewpoint(dt);
      return (transform.Position, calculated_velocity);
    }

    public void Look(float dt) {
      Vec2 rel_pos = Mouse.RelativePosition;
      float new_yaw = player_viewpoint.Yaw + (rel_pos.x * player_viewpoint.Sensitivity);
      float new_pitch = player_viewpoint.Pitch - (rel_pos.y * player_viewpoint.Sensitivity);

      player_viewpoint.Yaw = new_yaw;
      
      /// dont change if looking too far down because then player can see through themselves
      if (!pitch_interval.Contains(new_pitch)) {
        new_pitch = pitch_interval.ClampToClosestBound(new_pitch);
      }
      player_viewpoint.Pitch = new_pitch;

      player_viewpoint.CalculateMatrix();
      
      /// we want to project this forward vector down or up onto the plane of the players viewpoint
      /// so that we can get the right vector otherwise the player will fly away if lookup up
      
      Vec3 forward_projection = player_viewpoint.Forward - (Vec3.Dot(player_viewpoint.Forward, Vec3.up)) * Vec3.up; 
      Forward = forward_projection.Normalized();

      Right = player_viewpoint.Right.Normalized();
    }

    private void MoveViewpoint(float dt) {
      // Vec3 move = Vec3.zero;
      // Vec3 forward_velocity = Forward.Normalized() * Speed;
      // Vec3 right_velocity = Right.Normalized() * Speed;

      // if (Input.GetKey(KeyCode.W)) {
      //   move += forward_velocity;
      // } else if (Input.GetKey(KeyCode.S)) {
      //   move -= forward_velocity;
      // }

      // if (Input.GetKey(KeyCode.D)) {
      //   move += right_velocity;
      // } else if (Input.GetKey(KeyCode.A)) {
      //   move -= right_velocity;
      // }

      // Vec3 movement = move * Speed * dt;
      // linear_velocity += movement;
      // if (linear_velocity.Magnitude() > MaxSpeed) {
      //   linear_velocity = linear_velocity.Normalized() * MaxSpeed;
      // }

      // transform.Position = transform.Position + linear_velocity;
    }

    private void UpdateVelocity(float dt, MovementDirection direction, bool jump_key, bool crouch_key) {
      /// FIXME: have to fix the falloff on velocity, it doesn't change direction very well, rn we zero the velocity every frame
      ///         which is not the best way to do it
      Vec3 move = Vec3.zero;
      Vec3 forward_velocity = Forward.Normalized() * Speed;
      Vec3 right_velocity = Right.Normalized() * Speed;

      if (direction.HasFlag(MovementDirection.Forward)) {
        move += forward_velocity;
      } else if (direction.HasFlag(MovementDirection.Backward)) {
        move -= forward_velocity;
      }

      if (direction.HasFlag(MovementDirection.Right)) {
        move += right_velocity;
      } else if (direction.HasFlag(MovementDirection.Left)) {
        move -= right_velocity;
      }

      Vec3 jump_adjustment = Vec3.zero;
      if (jump_key) {
        if (current_jump_height < max_jump_height) {
          jump_adjustment = (new Vec3(0.0f, jump_speed, 0.0f)) * dt;
          current_jump_height += jump_speed * dt;
        }
      } else {
        current_jump_height = 0.0f;
      }

      Vec3 movement = move * Speed * dt;
      linear_velocity += movement;
      if (linear_velocity.Magnitude() > MaxSpeed) {
        linear_velocity = linear_velocity.Normalized() * MaxSpeed;
      }

      transform.Position = transform.Position + linear_velocity;
    }

    private bool InControllableState() {
      return state == MovementState.Idle || state == MovementState.Walking ||
             state == MovementState.Running || state == MovementState.Crouching;
    }

    private bool Transition(MovementEvent e) {
      MovementState new_state;
      if (!state_transitions.TryGetValue((state, e), out new_state)) {
        return false;
      }

      if (on_state_change != null) {
        on_state_change(e, state, new_state);
      }
      state = new_state;

      return true;
    }

    private void Initialize(Vec3? gravity = null) {
      if (gravity.HasValue) {
        this.gravity = gravity.Value;
      } else {
        this.gravity = new Vec3(0.0f, -9.8f, 0.0f);
      }
      linear_velocity = Vec3.zero;
      acceleration = Vec3.zero;
      max_speed = 5.0f;
      max_acceleration = 10.0f;
      jump_speed = 5.0f;
      current_jump_height = 0.0f;
      max_jump_height = 2.0f;
      crouch_speed = 2.0f;
      
      state = MovementState.Idle;
      state_transitions = new Dictionary<(MovementState, MovementEvent), MovementState>();

      SetTransitions();
    }

    private void SetTransitions() {
      AddTransition(MovementEvent.BeginWalking, MovementState.Idle, MovementState.Walking);
      AddTransition(MovementEvent.StopWalking, MovementState.Walking, MovementState.Idle);

      AddTransition(MovementEvent.StopRunning, MovementState.Running, MovementState.Idle);
      AddTransition(MovementEvent.BeginRunningFromIdle, MovementState.Idle, MovementState.Running);
      AddTransition(MovementEvent.BeginRunningFromWalking, MovementState.Walking, MovementState.Running);
    }

    private void AddTransition(MovementEvent e, MovementState from, MovementState to) {
      state_transitions.Add((from, e), to);
    }

    public Vec3 Gravity {
      set { gravity = value; }
    }

    public Vec3 WorldPosition {
      get { return transform.Position; }
      set { transform.Position = value; }
    }

    public Vec3 LinearVelocity {
      get { return linear_velocity; }
      set { linear_velocity = value; }
    }

    public Vec3 Acceleration {
      get { return acceleration; }
      set { acceleration = value; }
    }

    public Vec3 Forward {
      get { return forward; }
      set { forward = value; }
    }

    public Vec3 Right {
      get { return right; }
      set { right = value; }
    }

    public float Speed {
      get { return current_speed; }
      set { current_speed = value; }
    }

    public float WalkSpeed {
      get { return walk_speed; }
      set { walk_speed = value; }
    }

    public float RunSpeed {
      get { return run_speed; }
      set { run_speed = value; }
    }

    public float CrouchSpeed {
      get { return crouch_speed; }
      set { crouch_speed = value; }
    }

    public float MaxSpeed {
      get { return max_speed; }
      set { max_speed = value; }
    }

    public float MaxAcceleration {
      get { return max_acceleration; }
      set { max_acceleration = value; }
    }

    public float JumpSpeed {
      get { return jump_speed; }
      set { jump_speed = value; }
    }

    public float CurrentJumpHeight {
      get { return current_jump_height; }
      set { current_jump_height = value; }
    }

    public float MaxJumpHeight {
      get { return max_jump_height; }
      set { max_jump_height = value; }
    }

    public MovementState State {
      get { return state; }
    }
  }
}
