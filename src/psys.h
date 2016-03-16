/*
alphaclock - transparent desktop clock
Copyright (C) 2016  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef PSYS_H_
#define PSYS_H_

#include <vector>
#include "vec3.h"
#include "image.h"

struct PSysParam {
	// emitter parameters
	float spawn_rate;
	float spawn_range;
	float life, life_range;
	float size, size_range;
	Vec3 gravity;
	Image *spawn_map;
	float spawn_map_speed;

	// particle parameters
	Image *pimg;
	Vec3 pcolor_start, pcolor_mid, pcolor_end;
	float palpha_start, palpha_mid, palpha_end;
	float pscale_start, pscale_mid, pscale_end;
};

void psys_default(PSysParam *pp);

struct Particle {
	Vec3 pos, vel;
	Vec3 color;
	float alpha;
	float life, max_life;
	float size, scale;

	struct Particle *next;
};

class ParticleSystem {
private:
	float spawn_pending;
	Particle *plist;
	int pcount;

	float active_time;
	bool expl;
	Vec3 expl_cent;
	float expl_force, expl_dur;
	float expl_life;

	Vec3 *smcache;
	int smcache_max[256];
	void gen_spawnmap(int count);

	void spawn_particle();

public:
	Vec3 pos;
	PSysParam pp;
	bool active;

	ParticleSystem();
	~ParticleSystem();

	void reset();
	void reset_spawnmap();

	void explode(const Vec3 &c, float force, float dur = 1.0, float life = 0.0);

	bool alive() const;

	void update(float dt);
	void draw() const;
};

#endif	// PSYS_H_
