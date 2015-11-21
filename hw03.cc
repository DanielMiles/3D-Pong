/// LSU EE 4702-1 (Fall 2015), GPU Programming
//
 /// Homework 3
 //
 /// Your Name:

 /// Instructions
 //
 //  Read the assignment: http://www.ece.lsu.edu/koppel/gpup/2015/hw03.pdf


/// Purpose
//
//   Demonstrate simulation of point masses connected by springs.


/// What Code Does

// Simulates balls connected by springs over a platform. Balls and
// springs can be initialized in different arrangements (called
// scenes). Currently scene 1 is a simple string of beads, and scenes
// 2, 3, and 4 are trusses. The platform consists of tiles, some are
// purple-tinted mirrors (showing a reflection of the ball), the
// others show the course syllabus.



///  Keyboard Commands
 //
 /// Object (Eye, Light, Ball) Location or Push
 //   Arrows, Page Up, Page Down
 //        Move object or push ball, depending on mode.
 //        With shift key pressed, motion is 5x faster.
 //   'e': Move eye.
 //   'l': Move light.
 //   'b': Move head (first) ball. (Change position but not velocity.)
 //   'B': Push head ball. (Add velocity.)
 //
 /// Eye Direction
 //   Home, End, Delete, Insert
 //   Turn the eye direction.
 //   Home should rotate eye direction up, End should rotate eye
 //   down, Delete should rotate eye left, Insert should rotate eye
 //   right.  The eye direction vector is displayed in the upper left.

 /// Simulation Options
 //  (Also see variables below.)
 //
 //  'c'    Clean the platform.
 //  'w'    Twirl balls around axis formed by head and tail.
 //  '1'    Set up scene 1.
 //  '2'    Set up scene 2.
 //  '3'    Set up scene 3.
 //  'p'    Pause simulation. (Press again to resume.)
 //  ' '    (Space bar.) Advance simulation by 1/30 second.
 //  'S- '  (Shift-space bar.) Advance simulation by one time step.
 //  'h'    Freeze position of first (head) ball. (Press again to release.)
 //  't'    Freeze position of last (tail) ball. (Press again to release.)
 //  's'    Stop balls.
 //  'g'    Turn gravity on and off.
 //  'y'    Toggle value of opt_tryout1. Intended for experiments and debugging.
 //  'Y'    Toggle value of opt_tryout2. Intended for experiments and debugging.
 //  'F11'  Change size of green text (in upper left).
 //  'F12'  Write screenshot to file.

 /// Variables
 //   Selected program variables can be modified using the keyboard.
 //   Use "Tab" to cycle through the variable to be modified, the
 //   name of the variable is displayed next to "VAR" on the bottom
 //   line of green text.

 //  'Tab' Cycle to next variable.
 //  '`'   Cycle to previous variable.
 //  '+'   Increase variable value.
 //  '-'   Decrease variable value.
 //
 //  VAR Spring Constant - Set spring constant.
 //  VAR Time Step Duration - Set physics time step.
 //  VAR Air Resistance - Set air resistance.
 //  VAR Light Intensity - The light intensity.
 //  VAR Gravity - Gravitational acceleration. (Turn on/off using 'g'.)


#define GL_GLEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

#include <gp/util.h>
#include <gp/glextfuncs.h>
#include <gp/coord.h>
#include <gp/shader.h>
#include <gp/pstring.h>
#include <gp/misc.h>
#include <gp/gl-buffer.h>
#include <gp/texture-util.h>
#include <gp/colors.h>

#include "util-containers.h"
#include "shapes.h"


///
/// Main Data Structures
///
//
// class World: All data about scene.


class World;


class Platform_Overlay {
public:
  Platform_Overlay():data(NULL){}

  // Note: When using the array below as an argument to glTexImage2D
  // use data format GL_RGBA and type GL_FLOAT.
  //
  pColor *data;
  GLuint txid;

  bool texture_modified;
  bool texture_object_initialized;

};

// Object Holding Ball State
//
class Ball {
public:
  Ball():velocity(pVect(0,0,0)),locked(false),
         color(color_lsu_spirit_gold),contact(false){};
  pCoor position;
  pVect velocity;

  float mass;
  float mass_min; // Mass below which simulation is unstable.
  float radius;

  bool locked;

  pVect force;
  pColor color;
  bool contact;                 // When true, ball rendered in gray.
  float spring_constant_sum;    // Used to compute minimum mass.

  void push(pVect amt);
  void translate(pVect amt);
  void stop();
  void freeze();
};

class Link {
public:
  Link(Ball *b1, Ball *b2):ball1(b1),ball2(b2),
     distance_relaxed(pDistance(b1->position,b2->position)), snapped(false),
     natural_color(color_lsu_spirit_purple),color(color_lsu_spirit_purple){}
  Link(Ball *b1, Ball *b2, pColor colorp):ball1(b1),ball2(b2),
     distance_relaxed(pDistance(b1->position,b2->position)), snapped(false),
     natural_color(colorp),color(colorp){}
  Ball* const ball1;
  Ball* const ball2;
  float distance_relaxed;
  bool snapped;
  pColor natural_color;
  pColor color;
};

class Platform{
public:
  Platform(pCoor tl, pCoor tr, pCoor br, pCoor bl, pColor c = pColor(0,0,1)){
    top_left = tl;
    top_right = tr;
    bot_right = br;
    bot_left = bl;
    color = c;
    natural_color = color;
  }
  Platform(){}
  void render(){
    glBegin(GL_QUADS);
    glColor3fv(color);
    glVertex3fv(top_left);
    glVertex3fv(top_right);
    glVertex3fv(bot_right);
    glVertex3fv(bot_left);
    glEnd();
  }
  pCoor top_left;
  pCoor top_right;
  pCoor bot_right;
  pCoor bot_left;
  pColor color;
  pColor natural_color;
};

class Cube{
public:
  Cube(pCoor pos, float s){
    position = pos;
    size = s;
    tb_left = position + pCoor(-size, size, -size);
    tb_right = position +  pCoor(size, size, -size);
    tf_left = position + pCoor(-size, size, size);
    tf_right = position + pCoor(size, size, size);
    bb_left = position + pCoor(-size, -size, -size);
    bb_right = position + pCoor(size, -size, -size);
    bf_left = position + pCoor(-size, -size, size);
    bf_right = position + pCoor(size, -size, size);
    platforms[0] = Platform(tf_left, tf_right, bf_right, bf_left);
    platforms[1] = Platform(tf_right, tb_right, bb_right, bf_right);
    platforms[2] = Platform(tb_left, tb_right, bb_right, bb_left);
    platforms[3] = Platform(tb_left, tf_left, bf_left, bb_left);
    platforms[4] = Platform(tb_left, tb_right, tf_right, tf_left);
    platforms[5] = Platform(bb_left, bb_right, bf_right, bf_left);

    paddle0_tl = position + pCoor(-size, 10, -10);
    paddle0_tr = position + pCoor(-size, 10, 10);
    paddle0_br = position + pCoor(-size, -10, 10);
    paddle0_bl = position + pCoor(-size, -10, -10);
    paddles[0] = Platform(paddle0_tl+pCoor(1,0,0), paddle0_tr+pCoor(1,0,0) , paddle0_br+pCoor(1,0,0), paddle0_bl+pCoor(1,0,0), pColor(1,0,0));
    paddles[1] = Platform(paddle0_tl+pCoor(2*size,0,0)+pCoor(-1,0,0), paddle0_tr+pCoor(2*size,0,0)+pCoor(-1,0,0), paddle0_br+pCoor(2*size,0,0)+pCoor(-1,0,0), paddle0_bl+pCoor(2*size,0,0)+pCoor(-1,0,0), pColor(1,0,0));
  }
  void render(){
    for(int i = 0; i < 6; i++){
      Platform pl = platforms[i];
      pl.render();
    }
    for (int i = 0; i < 2; i++){
      Platform pl = paddles[i];
      pl.render();
    }
  }
  bool checkCollision(Ball* ball){
    bool fail = false;    
    pCoor pos = ball->position;
    if (pos.x-ball->radius < bb_left.x || pos.x+ball->radius > tf_right.x) {fail = true; ball->velocity.x *= -1;}
    if (pos.y-ball->radius < bb_left.y || pos.y+ball->radius > tf_right.y) {fail = true; ball->velocity.y *= -1;}
    if (pos.z-ball->radius < bb_left.z || pos.z+ball->radius > tf_right.z) {fail = true; ball->velocity.z *= -1;}
    return fail;
 }

  Platform platforms[6];
  Platform paddles[2];
  float size;
  Ball* ball;
  pCoor position;
  pCoor tb_left;
  pCoor tb_right;
  pCoor tf_left;
  pCoor tf_right;
  pCoor bb_left;
  pCoor bb_right;
  pCoor bf_left;
  pCoor bf_right;
};

 /// Homework 3 All Problems
//
//   Use this class to define variables and member functions.
//   Don't modify hw03-graphics.cc.
//
class My_Piece_Of_The_World {
public:
  My_Piece_Of_The_World(World& wp):w(wp){};
  World& w;
  Platform_Overlay* platform_overlays;  // Array of overlays.
  Platform_Overlay sample_overlay;
  int nx, nz;          // Number of overlays along each dimension.
  int num_overlays;
  int twid_x, twid_z;  // Dimensions of each texture in texels.
  int num_texels;
  float wid_x, wid_z;  // Width of each overlay in object space units.
  float wid_x_inv, wid_z_inv;  // Their inverses.

  // Minimum x- and z- object space coordinate for most recent overlay.
  float overlay_xmin, overlay_zmin;

  void init();
  void sample_tex_make();

  // Return the platform overlay that includes pos, or NULL if pos is
  // not on platform.
  //
  Platform_Overlay* po_get(pCoor pos);

  //
  // Homework 3: These are suggested functions.
  //

  // Convert object space coordinate to texel coordinate relative
  // to overlay po.
  pCoor po_get_lcoor(Platform_Overlay *po, pCoor pos);

  // Return the texel at lpos in po.
  pColor* po_get_texel(Platform_Overlay *po, pCoor lpos);

  //
  // Homework 3: These functions must be implemented.
  //
  void render();
  void clean();
  
  float const size = 50;
  Cube cube = Cube(pCoor(0,size+5,0), size);
};

// Declare containers and iterators for Balls and Links.
// (See util_container.h.)
//
typedef pVectorI<Link> Links;
typedef pVectorI_Iter<Link> LIter;
typedef pVectorI<Ball> Balls;
typedef pVectorI_Iter<Ball> BIter;

struct Truss_Info {

  // See make_truss for a description of what the members are for.

  // Inputs
  PStack<pCoor> base_coors;  // Coordinates of first set of balls.
  pVect unit_length;
  int num_units;

  // Output
  Balls balls;
  Links links;
};

#include "hw03-graphics.cc"

void
My_Piece_Of_The_World::init()
{
  twid_x = 256; twid_z = 256;
  num_texels = twid_x * twid_z;
  nx = 40; nz = 40;
  num_overlays = nx * nz;
  sample_tex_make();

  platform_overlays = new Platform_Overlay[num_overlays];

  wid_x = ( w.platform_xmax - w.platform_xmin ) / nx;
  wid_z = ( w.platform_zmax - w.platform_zmin ) / nz;
  wid_x_inv = 1.0 / wid_x;
  wid_z_inv = 1.0 / wid_z;

}

void
My_Piece_Of_The_World::sample_tex_make()
{
  /// Homework 3 -- Sample Code

  // Code in this routine creates a texture with a big red X, and
  // loads it into a texture object.  The texture object can
  // be used as a substitute for the "scuffed" texture before
  // the scuffed texture part of this assignment is finished.

  Platform_Overlay* const po = &sample_overlay;

  // Allocate storage for the array of texels.
  //
  if ( !po->data ) po->data = new pColor[ num_texels ];

  // Initialize the texels to black and transparent. (Alpha = 0)
  //
  memset(po->data,0,num_texels*sizeof(po->data[0]));

  // Thickness of the strokes making up the letter ex.
  //
  const int thickness = max(2, twid_x/10);

  // Write the letter ex, a big one, in the texture.
  //
  for ( int tx=0; tx<twid_x-thickness; tx++ )
    {
      // Note: Compute tz without assuming twid_x == twid_z.
      int tz_raw = float(tx)/twid_x * twid_z;
      int tz = min(tz_raw,twid_z-1);

      // Array index of texel at (tx,tz).
      //
      int idx = tx + tz * twid_x;
      int idx2 = twid_x - 1 - tx + tz * twid_x;

      // Write colors to texels.
      //
      for ( int i=0; i<thickness; i++ )
        {
          po->data[idx+i] = color_red;
          po->data[idx+i].a = 1;
          po->data[idx2-i] = color_red;
          po->data[idx2-i].a = 1;
        }
    }

  // Create a new texture object.
  //
  glGenTextures(1,&po->txid);

  // Load our texture into the texture object.
  //
  glBindTexture(GL_TEXTURE_2D,po->txid);

  // Tell OpenGL to generate MIPMAP levels for us.
  //
  glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, 1);

  glTexImage2D
    (GL_TEXTURE_2D,
     0,                // Level of Detail (0 is base).
     GL_RGBA,          // Internal format to be used for texture.
     twid_x, twid_z,
     0,                // Border
     GL_RGBA,     // GL_BGRA: Format of data read by this call.
     GL_FLOAT,    // Size of component.
     (void*)po->data);
}


pColor*
My_Piece_Of_The_World::po_get_texel(Platform_Overlay *po, pCoor lpos)
{
  const int idx = 0; // Replace with a function of lpos, etc.
  if ( idx < 0 || idx >= num_texels ) return NULL;
  return &po->data[ idx ];
}

pCoor
My_Piece_Of_The_World::po_get_lcoor(Platform_Overlay *po, pCoor pos)
{
  pCoor lc;
  // Replace with function of pos, etc.
  lc.x = 0;
  lc.z = 0;
  lc.y = 0;
  lc.w = 0;
  return lc;
}

Platform_Overlay*
My_Piece_Of_The_World::po_get(pCoor pos)
{
  const int x = ( pos.x - w.platform_xmin ) * wid_x_inv;
  if ( x < 0 || x >= nx ) return NULL;
  const int z = ( pos.z - w.platform_zmin ) * wid_z_inv;
  if ( z < 0 || z >= nz ) return NULL;
  overlay_xmin = w.platform_xmin + x * wid_x;
  overlay_zmin = w.platform_zmin + z * wid_z;

  Platform_Overlay* const po = &platform_overlays[x + z * nz];

  if ( !po->data )
    {
      // This overlay has never been visited, initialize texel array.
      // po->data = umm.;
    }

  return po;
}

void
My_Piece_Of_The_World::render()
{
  /// Homework 3 -- Lots of stuff in this routine.

  // [ ] Enable at least some of the following:
  //     -- Texturing.
  //     -- Texture application mode.
  //     -- The Alpha Test
  //     -- Blending.
  //
  // See demo-8-texture.cc for examples.

  cube.render();

  for ( int i=0; i<num_overlays; i++ )
    {
      Platform_Overlay* const po = &platform_overlays[i];
      if ( !po->data ) continue;

      if ( !po->texture_object_initialized )
        {
          po->texture_object_initialized = true;

          // [ ] Do something here.
        }

      if ( po->texture_modified )
        {
          po->texture_modified = false;

          // [ ] Send texel array (po->data) to OpenGL.

        }

      // [ ] Render primitive(s) matching shape of overlay.
    }

  // [ ] Disable anything turned on at the start.

}

void
My_Piece_Of_The_World::clean()
{
  /// Homework 3:  [ ] Remove scuffs from dirty textures.

}


void
World::init()
{
  chain_length = 14;

  opt_time_step_duration = 0.0003;
  variable_control.insert(opt_time_step_duration,"Time Step Duration");

  distance_relaxed = 15.0 / chain_length;
  opt_spring_constant = 15000;
  variable_control.insert(opt_spring_constant,"Spring Constant");

  opt_gravity_accel = 0;
  opt_gravity = true;
  gravity_accel = pVect(0,-opt_gravity_accel,0);
  variable_control.insert(opt_gravity_accel,"Gravity");

  opt_air_resistance = 0;
  variable_control.insert(opt_air_resistance,"Air Resistance");

  world_time = 0;
  time_step_count = 0;
  last_frame_wall_time = time_wall_fp();
  frame_timer.work_unit_set("Steps / s");

  ball_eye = NULL;
  opt_ride = false;

  init_graphics();

  mp.init();

  ball_setup_1();
  lock_update();
}

Ball*
World::make_marker(pCoor position, pColor color)
{
  Ball* const ball = new Ball;
  ball->position = position;
  ball->locked = true;
  ball->velocity = pVect(0,0,0);
  ball->radius = 0.2;
  ball->mass = 0;
  ball->contact = false;
  ball->color = color;
  return ball;
}

void
World::lock_update()
{
  // This routine called when options like opt_head_lock might have
  // changed.

  // Update locked status.
  //
  if ( head_ball ) head_ball->locked = opt_head_lock;
  if ( tail_ball ) tail_ball->locked = opt_tail_lock;

  // Re-compute minimum mass needed for stability.
  //
  for ( BIter ball(balls); ball; ) ball->spring_constant_sum = 0;
  const double dtis = pow( opt_time_step_duration, 2 );
  for ( LIter link(links); link; )
    {
      Ball* const b1 = link->ball1;
      Ball* const b2 = link->ball2;
      b1->spring_constant_sum += opt_spring_constant;
      b2->spring_constant_sum += opt_spring_constant;
    }
  for ( BIter ball(balls); ball; )
    ball->mass_min = ball->spring_constant_sum * dtis;
}

void
World::balls_twirl()
{
  if ( !head_ball || !tail_ball ) return;

  pNorm axis(head_ball->position, tail_ball->position);

  for ( BIter ball(balls); ball; )
    {
      pVect b_to_top(ball->position,head_ball->position);
      const float dist_along_axis = dot(b_to_top,axis);
      const float lsq = b_to_top.mag_sq() - dist_along_axis * dist_along_axis;
      if ( lsq <= 1e-5 ) { ball->velocity = pVect(0,0,0); continue; }
      const float dist_to_axis = sqrt(lsq);
      pNorm rot_dir = cross(b_to_top,axis);
      ball->velocity += 10 * dist_to_axis * rot_dir;
    }
}

void
World::objects_erase()
{
  ball_eye = NULL;
  balls.erase();
  links.erase();
}

///
/// Physical Simulation Code
///

 /// Initialize Simulation
//

void
World::make_truss(Truss_Info *truss_info)
{
}

void
World::ball_setup_1()
{
  
  // Arrange and size balls to form a pendulum.

  pCoor first_pos(13.4,14,-9.2);
  pVect delta_pos = pVect(distance_relaxed,0,0);

  // Remove objects from the simulated objects lists, balls and links.
  // The delete operator is used on objects in the lists.
  //
  objects_erase();
  Ball* ball =  new Ball();
  ball->position = first_pos;
  ball->locked = false;
  ball->velocity = pVect(25,25,25);
  ball->radius = 1;
  ball->mass = 4/3.0 * M_PI * pow(ball->radius,3);
  ball->contact = false;
  ball->color = pColor(1,.25,1);
  balls += ball;
  mp.cube.ball = ball;

  // The balls pointed to by head_ball and tail_ball can be manipulated
  // using the user interface (by pressing 'h' or 't', for example).
  // Set these variables.
  //
  head_ball = ball;
  tail_ball = ball;

  opt_head_lock = false;    // Head ball will be frozen in space.
  opt_tail_lock = false;    // Tail ball can move freely.
}

void
World::ball_setup_2()
{
}

void
World::ball_setup_3()
{
}


void
World::ball_setup_4()
{
}

void
World::ball_setup_5()
{
}


 /// Advance Simulation State by delta_t Seconds
//
void
World::time_step_cpu(double delta_t)
{
  time_step_count++;

  // Smoothly move ball in response to user input.
  //
  if ( adj_t_stop )
    {
      const double dt = min(world_time,adj_t_stop) - adj_t_prev;
      pVect adj = dt/adj_duration * adj_vector;
      balls_translate(adj,0);
      adj_t_prev = world_time;
      if ( world_time >= adj_t_stop ) adj_t_stop = 0;
    }

  for ( BIter ball(balls); ball; )
    ball->force = ball->mass * gravity_accel;

  ///
  /// Update Position of Each Ball
  ///

  for ( BIter ball(balls); ball; )
    {
      if ( ball->locked )
        {
          ball->velocity = pVect(0,0,0);
          continue;
        }

      // Update Velocity
      //
      // This code assumes that force on ball is constant over time
      // step. This is clearly wrong when balls are moving with
      // respect to each other because the springs are changing
      // length. This inaccuracy will make the simulation unstable
      // when spring constant is large for the time step.
      //
      const float mass = max( ball->mass, ball->mass_min );

      pVect delta_v = ( ball->force / mass ) * delta_t;

      const float dist_above = ball->position.y - ball->radius;

      const bool collision =
        platform_collision_possible(ball->position) && dist_above < 0;

      
      if ( collision )
        {
          const float spring_constant_plat =
            ball->velocity.y < 0 ? 100000 : 50000;
          const float fric_coefficient = 0.1;
          const float force_up = -dist_above * spring_constant_plat;
          const float delta_v_up = force_up / mass * delta_t;
          const float fric_force_mag = fric_coefficient * force_up;
          pNorm surface_v(ball->velocity.x,0,ball->velocity.z);
          const float delta_v_surf = fric_force_mag / mass * delta_t;
          if ( delta_v_surf > surface_v.magnitude )
            {
              // Ignoring other forces?
              delta_v =
                pVect(-ball->velocity.x,delta_v.y,-ball->velocity.z);
            }
          else
            {
              delta_v -= delta_v_surf * surface_v;
            }
          delta_v.y += delta_v_up;
        }

      mp.cube.checkCollision(ball);

      ball->velocity += delta_v;

      // Air Resistance
      //
      const double fs = pow(1+opt_air_resistance,-delta_t);
      ball->velocity *= fs;

      // Update Position
      //
      // Assume that velocity is constant.
      //

      pCoor pos_prev = ball->position;

      ball->position += ball->velocity * delta_t;

      if ( !collision ) continue;

      /// Homework 3
      //
      //  [x] Retrieve the correct overlay.
      //  [ ] Determine area of platform that ball is touching.
      //  [ ] Convert to texel coordinate units.
      //  [ ] Modify texels in array.
      //  [ ] Set po->texture_modified iff any texels change.

      Platform_Overlay* const po = mp.po_get(ball->position);
      if ( !po ) continue;

      pCoor ball_lcor = mp.po_get_lcoor(po,ball->position);

    }
}

bool
World::platform_collision_possible(pCoor pos)
{
  // Assuming no motion in x or z axes.
  //
  return pos.x >= platform_xmin && pos.x <= platform_xmax
    && pos.z >= platform_zmin && pos.z <= platform_zmax;
}

 /// External Modifications to State
//
//   These allow the user to play with state while simulation
//   running.

// Move the ball.
//
void Ball::translate(pVect amt) {position += amt;}

// Add velocity to the ball.
//
void Ball::push(pVect amt) {velocity += amt;}

// Set the velocity to zero.
//
void Ball::stop() {velocity = pVect(0,0,0); }

// Set the velocity and rotation (not yet supported) to zero.
//
void Ball::freeze() {velocity = pVect(0,0,0); }



void World::balls_translate(pVect amt,int b){head_ball->translate(amt);}
void World::balls_push(pVect amt,int b){head_ball->push(amt);}
void World::balls_translate(pVect amt)
{ for ( BIter ball(balls); ball; ) ball->translate(amt); }
void World::balls_push(pVect amt)
{ for ( BIter ball(balls); ball; ) ball->push(amt); }
void World::balls_stop()
{ for ( BIter ball(balls); ball; ) ball->stop(); }
void World::balls_freeze(){balls_stop();}
void World::render_my_piece() {mp.render();}


void
World::frame_callback()
{
  // This routine called whenever window needs to be updated.

  const double time_now = time_wall_fp();

  if ( !opt_pause || opt_single_frame || opt_single_time_step )
    {
      /// Advance simulation state.

      // Amount of time since the user saw the last frame.
      //
      const double wall_delta_t = time_now - last_frame_wall_time;

      // Compute amount by which to advance simulation state for this frame.
      //
      const double duration =
        opt_single_time_step ? opt_time_step_duration :
        opt_single_frame ? 1/30.0 :
        wall_delta_t;

      const double world_time_target = world_time + duration;

      while ( world_time < world_time_target )
        {
          time_step_cpu(opt_time_step_duration);
          world_time += opt_time_step_duration;
        }

      // Reset these, just in case they were set.
      //
      opt_single_frame = opt_single_time_step = false;
    }

  last_frame_wall_time = time_now;

  if ( opt_ride && ball_eye )
    {
      pNorm b_eye_down(ball_eye->position,ball_down->position);
      pVect b_eye_up = -b_eye_down;
      pCoor eye_pos = ball_eye->position + 2.2 * ball_eye->radius * b_eye_up;
      pNorm b_eye_direction(eye_pos,ball_gaze->position);

      pNorm b_eye_left = cross(b_eye_direction,b_eye_up);
      pMatrix_Translate center_eye(-eye_pos);
      pMatrix rotate; rotate.set_identity();
      for ( int i=0; i<3; i++ ) rotate.rc(0,i) = b_eye_left.elt(i);
      for ( int i=0; i<3; i++ ) rotate.rc(1,i) = b_eye_up.elt(i);
      for ( int i=0; i<3; i++ ) rotate.rc(2,i) = -b_eye_direction.elt(i);
      modelview = rotate * center_eye;
      pMatrix reflect; reflect.set_identity(); reflect.rc(1,1) = -1;
      transform_mirror = modelview * reflect * invert(modelview);
    }

  render();
}

int
main(int argv, char **argc)
{
  pOpenGL_Helper popengl_helper(argv,argc);
  World world(popengl_helper);

  popengl_helper.rate_set(30);
  popengl_helper.display_cb_set(world.frame_callback_w,&world);
}

