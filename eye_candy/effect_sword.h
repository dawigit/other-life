/*!
 \brief Special effects for making magical swords sparkle as they're swung.
 */

#ifndef EFFECT_SWORD_H
#define EFFECT_SWORD_H

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"

namespace ec
{

	// C L A S S E S //////////////////////////////////////////////////////////////

	class SwordParticle : public Particle
	{
		public:
			SwordParticle(Effect* _effect, ParticleMover* _mover,
				const Vec3 _pos, const Vec3 _velocity, const coord_t _size,
				const alpha_t _alpha, const color_t red, const color_t green,
				const color_t blue, TextureEnum _texture, const Uint16 _LOD);
			~SwordParticle()
			{
			}

			virtual bool idle(const Uint64 delta_t);
			virtual Uint32 get_texture();
			virtual light_t estimate_light_level() const
			{
				return 0.0;
			}
			;
			virtual light_t get_light_level()
			{
				return 0.0;
			}
			;

			TextureEnum texture;
			Uint16 LOD;
	};

	class SwordEffect : public Effect
	{
		public:
			enum SwordType
			{
				SERPENT,
				CUTLASS,
				EMERALD_CLAYMORE,
				SUNBREAKER,
				ORC_SLAYER,
				EAGLE_WING,
				JAGGED_SABER,
				SWORD_OF_FIRE,
				SWORD_OF_ICE,
				SWORD_OF_MAGIC
			};

			SwordEffect(EyeCandy* _base, bool* _dead, Vec3* _start, Vec3* _end,
				const SwordType _type, const Uint16 _LOD);
			~SwordEffect();

			virtual EffectEnum get_type()
			{
				return EC_SWORD;
			}
			;
			bool idle(const Uint64 usec);
			virtual void request_LOD(const float _LOD);

			ParticleMover* mover;
			Vec3* start;
			Vec3* end;
			Vec3 old_end;
			coord_t size;
			alpha_t alpha;
			color_t color[3];
			TextureEnum texture;
			SwordType type;
	};

///////////////////////////////////////////////////////////////////////////////

} // End namespace ec

#endif	// defined EFFECT_SWORD_H
