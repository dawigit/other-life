#ifdef SFX

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_fountain.h"

namespace ec
{

extern MathCache_Lorange math_cache;

// C L A S S   F U N C T I O N S //////////////////////////////////////////////

FountainParticle::FountainParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _base_height, const bool _backlight, const float _sqrt_scale, const coord_t _max_size, const coord_t size_scalar) : Particle(_effect, _mover, _pos, _velocity)
{
  base_height = _base_height;
  backlight = _backlight;
  sqrt_scale = _sqrt_scale;
  max_size = _max_size;
  color[0] = 0.89;
  color[1] = 0.95;
  color[2] = 1.0;
  size = size_scalar * (0.5 + 5 * randcoord());
  alpha = sqrt_scale * 3.5 / size;
  if (backlight)
    alpha /= 9;
  if (alpha > 1.0)
    alpha = 1.0;
  flare_max = 1.5;
  flare_exp = 0.3;
  flare_frequency = 5.0;
  state = 0;
}

void FountainParticle::draw(const Uint64 usec)
{
  if (!backlight)
  {
    glEnable(GL_LIGHTING);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Vec3 shifted_pos = pos - *(((FountainEffect*)effect)->pos);
#if 0	// Clear, but slow.  Consider this a comment.
    Vec3 velocity2 = velocity;
//    velocity2.normalize();
    shifted_pos.normalize();
    Vec3 cross = velocity2.cross(shifted_pos).cross(velocity2);
    glNormal3f(cross.x, cross.y, cross.z);
#else	// Faster, but obfuscated.  NOTE: For some reason, it's not producing the same output!  However, I like the new output better.
    const coord_t vel_mag_squared = velocity.magnitude_squared();
    const coord_t sp_mag = shifted_pos.magnitude();
    const coord_t scalar = vel_mag_squared * sp_mag;
    const coord_t vx_squared = velocity.x * velocity.x;
    const coord_t vy_squared = velocity.y * velocity.y;
    const coord_t vz_squared = velocity.z * velocity.z;
    const coord_t x = (shifted_pos.x * (vz_squared - vy_squared) - velocity.x * (shifted_pos.z * velocity.z + shifted_pos.y * velocity.y)) / scalar;
    const coord_t y = (shifted_pos.y * (vx_squared - vz_squared) - velocity.y * (shifted_pos.x * velocity.x + shifted_pos.z * velocity.z)) / scalar;
    const coord_t z = (shifted_pos.z * (vy_squared - vx_squared) - velocity.z * (shifted_pos.y * velocity.y + shifted_pos.x * velocity.x)) / scalar;
    glNormal3f(-x, -y, -z);
#endif
  }
  Particle::draw(usec);
  if (!backlight)
  {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_LIGHTING);
  }
}

bool FountainParticle::idle(const Uint64 delta_t)
{
  if (effect->recall)
    return false;

  if (state == 0)
  {
    const float scalar = math_cache.powf_05_close((float)delta_t / 2500000);
    alpha *= scalar;
    size = size / scalar * 0.25 + size * 0.75;
    if (size >= max_size)
      size = max_size;

    if (pos.y < base_height)
    {
      state = 1;
      velocity.randomize(sqrt_scale);
      velocity.y = -velocity.y / 10;
      pos.y = base_height;
    }
  }
  else
  {
    if (alpha < 0.02)
      return false;
  
    const float scalar = math_cache.powf_05_close((float)delta_t / 200000);
    alpha *= scalar;
    size = size / scalar * 0.25 + size * 0.75;
    if (size >= max_size)
      size = max_size;
  }
  
  return true;
}

GLuint FountainParticle::get_texture(const Uint16 res_index)
{
  if (state == 1)
    return base->TexWater.get_texture(res_index);
  else
    return base->TexFlare.get_texture(res_index);
}

FountainEffect::FountainEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, const bool _backlight, const coord_t _base_height, const float _scale, const Uint16 _LOD)
{
  if (EC_DEBUG)
    std::cout << "FountainEffect (" << this << ") created." << std::endl;
  base = _base;
  dead = _dead;
  pos = _pos;
  count = 0;
  backlight = _backlight;
  scale = _scale;
  sqrt_scale = fastsqrt(scale);
  max_size = 3 * scale * 90 / (_LOD + 10);
  size_scalar = sqrt_scale * 4.5 / (_LOD + 5);
  base_height = _base_height + 0.1;
  count_scalar = 15000 / _LOD;
  LOD = _LOD;
  desired_LOD = _LOD;
  mover = new SimpleGravityMover(this);
  basic_mover = new ParticleMover(this);
  spawner = new FilledSphereSpawner(0.05 * sqrt_scale);

/*  
  for (int i = 0; i < LOD * 4; i++)
  {
    const Vec3 coords = spawner->get_new_coords() + *pos + Vec3(0.0, 0.1 * sqrt_scale, 0.0);
    Vec3 velocity;
    velocity.randomize(0.1 * sqrt_scale);
    velocity += Vec3(0.0, 0.8 * scale, 0.0);
    Particle* p = new FountainParticle(this, mover, coords, velocity, pos->y, backlight, sqrt_scale, max_size, size_scalar);
    if (!base->push_back_particle(p))
      break;
  }
*/
}

FountainEffect::~FountainEffect()
{
  delete mover;
  delete spawner;
  if (EC_DEBUG)
    std::cout << "FountainEffect (" << this << ") destroyed." << std::endl;
}

bool FountainEffect::idle(const Uint64 usec)
{
  if ((recall) && (particles.size() == 0))
    return false;
    
  if (recall)
    return true;
    
  count += usec;

  while (count > 0)
  {
    const Vec3 coords = spawner->get_new_coords() + *pos + Vec3(0.0, 0.1 * sqrt_scale, 0.0);
    Vec3 velocity;
    velocity.randomize(0.1 * sqrt_scale);
    velocity += Vec3(0.0, 1.0 * scale, 0.0);
    Particle* p = new FountainParticle(this, mover, coords, velocity, base_height, backlight, sqrt_scale, max_size, size_scalar);
    if (!base->push_back_particle(p))
    {
      count = 0;
      break;
    }
    count -= count_scalar;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////

};

#endif	// #ifdef SFX