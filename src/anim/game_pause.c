#include <stdlib.h>
#include <string.h>

#include "ll_led.h"
#include "ll_anim.h"

#include "anim/game_pause.h"

#define STEP_COUNT (31)
#define MIN_ALPHA (15)

enum step
{
	STEP_DOWN,
	STEP_UP
};
struct payload
{
	uint32_t step_count;
	uint32_t step_switch_count;
	uint8_t org_alpha[LL_LED_NUM_LEDS + 1];
	uint8_t alpha_step[LL_LED_NUM_LEDS + 1];
	uint8_t brightness_reference;
	uint8_t brightest_value;
	enum step step;
};

static uint32_t start_animation(struct color *framebuffer, void *payload)
{
	struct payload *p = payload;
	p->step = STEP_DOWN;
	p->step_count = 0;
	p->step_switch_count = 0;
	p->brightness_reference = 0;
	for (uint32_t i = 0; i < LL_LED_NUM_LEDS; ++i)
	{
		p->org_alpha[i] = framebuffer[i].a;
		p->alpha_step[i] = framebuffer[i].a / (uint8_t) STEP_COUNT;
		if(p->brightest_value < framebuffer[i].a )
		{
			p->brightest_value = framebuffer[i].a;
		}
	}
	p->alpha_step[LL_LED_NUM_LEDS] = p->brightest_value / (uint8_t) STEP_COUNT;
	p->brightness_reference = p->brightest_value;
	return 1;
}

static uint32_t run_animation(struct color *framebuffer, void *payload)
{
	static uint32_t wait_counter = 0;
	struct payload *p = payload;
	static uint32_t my_rand_value = 1;

	wait_counter++;
	if(wait_counter < 3)
	{
		return 1;
	}
	wait_counter = 0;
	if(p->brightness_reference <= MIN_ALPHA)
	{
		p->step = STEP_UP;
		if(my_rand_value < 100)
		{
			my_rand_value += 100;
		}
	}
	else if(p->brightness_reference >= p->brightest_value)
	{
		p->step = STEP_DOWN;
		if(my_rand_value < 100)
		{
			my_rand_value += 100;
		}
	}
	switch(p->step)
	{
		case STEP_DOWN:
			p->step_count++;
			for (uint32_t i = 0; i < LL_LED_NUM_LEDS; ++i)
			{
				framebuffer[i].a -= p->alpha_step[i];
			}
			p->brightness_reference -= p->alpha_step[LL_LED_NUM_LEDS];
			break;

		case STEP_UP:
			p->step_count++;
			for (uint32_t i = 0; i < LL_LED_NUM_LEDS; ++i)
			{
				framebuffer[i].a += p->alpha_step[i];
			}
			p->brightness_reference += p->alpha_step[LL_LED_NUM_LEDS];
			break;
	}
	return 1;
}

static uint32_t finish_animation(struct color *framebuffer, void *payload)
{
	struct payload *p = payload;
	if (p->brightness_reference == p->brightest_value)
	{
		return 1;
	}

	for (uint32_t i = 0; i < LL_LED_NUM_LEDS; ++i)
	{
		framebuffer[i].a += p->alpha_step[i];
	}
	p->brightness_reference += p->alpha_step[LL_LED_NUM_LEDS];

	return 0;
}

struct animation *anim_game_pause_init(struct color *framebuffer)
{
	struct animation      *anim = malloc(sizeof(*anim));
	static struct payload p;

	if (!anim)
		return NULL;

	anim->payload          = &p;
	anim->speed            = 90;
	anim->start_animation  = start_animation;
	anim->run_animation    = run_animation;
	anim->finish_animation = finish_animation;
	anim->framebuffer      = framebuffer;

	return anim;
}