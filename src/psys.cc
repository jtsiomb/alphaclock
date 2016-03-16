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
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include "opengl.h"
#include "psys.h"

#define MAX_SPAWNMAP_SAMPLES	2048

static double frand();
static float rndval(float x, float range);
static Particle *palloc();
static void pfree(Particle *p);
static void pfreelist(Particle *p);

void psys_default(PSysParam *pp)
{
	// default parameters
	pp->spawn_rate = 10.0;
	pp->spawn_range = 0.0;
	pp->life = 1.0;
	pp->life_range = 0.0;
	pp->size = 1.0;
	pp->size_range = 0.0;
	pp->spawn_map = 0;
	pp->spawn_map_speed = 0.0;

	pp->gravity = Vec3(0, -9.2, 0);

	pp->pimg = 0;
	pp->pcolor_start = pp->pcolor_mid = pp->pcolor_end = Vec3(1, 1, 1);
	pp->palpha_start = 1.0;
	pp->palpha_mid = 0.5;
	pp->palpha_end = 0.0;
	pp->pscale_start = pp->pscale_mid = pp->pscale_end = 1.0;
}

ParticleSystem::ParticleSystem()
{
	active = true;
	active_time = 0.0f;
	spawn_pending = 0.0f;
	plist = 0;
	pcount = 0;
	smcache = 0;

	expl = false;
	expl_force = expl_dur = 0.0f;
	expl_life = 0.0f;

	psys_default(&pp);
}

ParticleSystem::~ParticleSystem()
{
	pfreelist(plist);
	delete [] smcache;
}

void ParticleSystem::reset()
{
	pfreelist(plist);
	delete [] smcache;
	smcache = 0;

	active = true;
	active_time = 0.0f;
	spawn_pending = 0.0f;

	psys_default(&pp);
}

void ParticleSystem::reset_spawnmap()
{
	delete [] smcache;
	smcache = 0;
}

void ParticleSystem::explode(const Vec3 &c, float force, float dur, float life)
{
	if(expl) return;

	expl_dur = dur;
	expl_force = force;
	expl_cent = c;
	expl_life = life;
	expl = true;
}

bool ParticleSystem::alive() const
{
	return active || pcount > 0;
}

void ParticleSystem::update(float dt)
{
	if(pp.spawn_map && !smcache) {
		gen_spawnmap(MAX_SPAWNMAP_SAMPLES);
	}

	if(active) {
		active_time += dt;
	}

	if(expl) {
		expl = false;
		//active = false;

		Vec3 cent = expl_cent + pos;

		Particle *p = plist;
		while(p) {
			p->max_life = expl_dur;
			Vec3 dir = p->pos - cent;
			p->vel += (normalize(dir + Vec3((frand() - 0.5) * 0.5, frand() - 0.5, (frand() - 0.5) * 0.5))) * expl_force;
			p = p->next;
		}
	}

	if(expl_life > 0.0) {
		expl_life -= dt;
		if(expl_life <= 0.0) {
			expl_life = 0.0;
			active = false;
		}
	}

	// update active particles
	Particle *p = plist;
	while(p) {
		p->life += dt;
		if(p->life < p->max_life) {
			float t = p->life / p->max_life;

			p->pos = p->pos + p->vel * dt;
			p->vel = p->vel + pp.gravity * dt;

			if(t < 0.5) {
				t *= 2.0;
				p->color = lerp(pp.pcolor_start, pp.pcolor_mid, t);
				p->alpha = lerp(pp.palpha_start, pp.palpha_mid, t);
				p->scale = lerp(pp.pscale_start, pp.pscale_mid, t);
			} else {
				t = (t - 0.5) * 2.0;
				p->color = lerp(pp.pcolor_mid, pp.pcolor_end, t);
				p->alpha = lerp(pp.palpha_mid, pp.palpha_end, t);
				p->scale = lerp(pp.pscale_mid, pp.pscale_end, t);
			}

		} else {
			p->life = -1.0;
		}
		p = p->next;
	}

	// remove dead particles
	Particle dummy;
	dummy.next = plist;
	p = &dummy;
	while(p->next) {
		if(p->next->life < 0.0) {
			Particle *tmp = p->next;
			p->next = tmp->next;
			pfree(tmp);
			--pcount;
		} else {
			p = p->next;
		}
	}
	plist = dummy.next;

	float spawn_rate = pp.spawn_rate;
	if(pp.spawn_map && pp.spawn_map_speed > 0.0) {
		float s = active_time * pp.spawn_map_speed;
		if(s > 1.0) s = 1.0;
		spawn_rate *= s;
	}

	// spawn particles as needed
	if(active) {
		spawn_pending += spawn_rate * dt;

		while(spawn_pending >= 1.0f) {
			spawn_pending -= 1.0f;
			spawn_particle();
		}
	}
}

void ParticleSystem::draw() const
{
	int cur_sdr = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &cur_sdr);
	if(cur_sdr) {
		glUseProgram(0);
	}

	glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	if(pp.pimg) {
		if(!pp.pimg->texture) {
			pp.pimg->gen_texture();
		}
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, pp.pimg->texture);
	}

	glBegin(GL_QUADS);
	Particle *p = plist;
	while(p) {
		float hsz = p->size * p->scale * 0.5;
		glColor4f(p->color.x, p->color.y, p->color.z, p->alpha);
		glTexCoord2f(0, 0); glVertex3f(p->pos.x - hsz, p->pos.y - hsz, p->pos.z);
		glTexCoord2f(1, 0); glVertex3f(p->pos.x + hsz, p->pos.y - hsz, p->pos.z);
		glTexCoord2f(1, 1); glVertex3f(p->pos.x + hsz, p->pos.y + hsz, p->pos.z);
		glTexCoord2f(0, 1); glVertex3f(p->pos.x - hsz, p->pos.y + hsz, p->pos.z);
		p = p->next;
	}
	glEnd();

	glPopAttrib();

	if(cur_sdr) {
		glUseProgram(cur_sdr);
	}
}

void ParticleSystem::gen_spawnmap(int count)
{
	Image *img = pp.spawn_map;
	if(!img) return;

	delete [] smcache;
	smcache = new Vec3[count];

	float umax = (float)img->width;
	float vmax = (float)img->height;
	float aspect = umax / vmax;

	// first generate a bunch of random samples by rejection sampling
	for(int i=0; i<count; i++) {
		float u, v;
		unsigned char val, ord;

		do {
			u = (double)rand() / (double)RAND_MAX;
			v = (double)rand() / (double)RAND_MAX;

			int x = (int)(u * umax);
			int y = (int)(v * vmax);

			unsigned char *pptr = img->pixels + (y * img->width + x) * (img->bpp / 8);
			val = pptr[0];
			ord = pptr[1];
		} while(val < 192);

		smcache[i] = Vec3(u * 2.0 - 1.0, (1.0 - v * 2.0) / aspect, ord / 255.0);
	}

	// then order by z
	std::sort(smcache, smcache + count,
			[](const Vec3 &a, const Vec3 &b) { return a.z < b.z; });

	// precalculate the bounds of each slot
	smcache_max[0] = 0;
	for(int i=1; i<255; i++) {
		float maxval = (float)i / 255.0;

		int idx = smcache_max[i - 1];
		while(++idx < count && smcache[idx].z < maxval);
		smcache_max[i] = idx;
	}
	smcache_max[255] = count;
}

static double frand()
{
	return (double)rand() / (double)RAND_MAX;
}

static float rndval(float x, float range)
{
	if(fabs(range) < 1e-6) {
		return x;
	}
	return x + (frand() * range - range * 0.5);
}

void ParticleSystem::spawn_particle()
{
	Particle *p = palloc();
	p->pos = Vec3(rndval(pos.x, pp.spawn_range),
			rndval(pos.y, pp.spawn_range),
			rndval(pos.z, pp.spawn_range));
	p->vel = Vec3(0, 0, 0);
	p->color = pp.pcolor_start;
	p->alpha = pp.palpha_start;
	p->life = 0.0;
	p->max_life = rndval(pp.life, pp.life_range);
	p->size = rndval(pp.size, pp.size_range);
	p->scale = pp.pscale_start;

	if(pp.spawn_map) {
		float maxz = pp.spawn_map_speed > 0.0 ? active_time * pp.spawn_map_speed : 1.0;
		int max_idx = (int)(maxz * 255.0);
		if(max_idx > 255) max_idx = 255;
		if(max_idx < 1) max_idx = 1;

		int idx = rand() % smcache_max[max_idx];

		p->pos.x += smcache[idx].x;
		p->pos.y += smcache[idx].y;
	}

	p->next = plist;
	plist = p;
	++pcount;
}

// particle allocator
#define MAX_POOL_SIZE	8192
static Particle *ppool;
static int ppool_size;

static Particle *palloc()
{
	if(ppool) {
		Particle *p = ppool;
		ppool = ppool->next;
		--ppool_size;
		return p;
	}
	return new Particle;
}

static void pfree(Particle *p)
{
	if(!p) return;

	if(ppool_size < MAX_POOL_SIZE) {
		p->next = ppool;
		ppool = p;
		++ppool_size;
	} else {
		delete p;
	}
}

static void pfreelist(Particle *p)
{
	if(!p) return;

	Particle *it = p;
	int new_pool_size = ppool_size;

	while(it->next && new_pool_size < MAX_POOL_SIZE) {
		it = it->next;
		++new_pool_size;
	}

	Particle *last = it;
	it = it->next;

	// add the first lot to the pool
	last->next = ppool;
	ppool = p;
	ppool_size = new_pool_size;

	// delete the rest;
	while(it) {
		p = it;
		it = it->next;
		delete p;
	}
}
