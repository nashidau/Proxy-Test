
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>


static void smart_add(Evas_Object *);
static void smart_move(Evas_Object *, int , int);

static const Evas_Smart_Class smart_class = {
	.name = "SmartProxyClass",
	.version = EVAS_SMART_CLASS_VERSION,
	.add = smart_add,
	.move = smart_move
};

Evas_Smart *smart;

Evas_Object *bg;
Evas_Object *text;
int state = 0;


Evas_Object *
sp_add(Evas *e){

	if (!smart)
		smart = evas_smart_class_new(&smart_class);
	return evas_object_smart_add(e, smart);
}

void
sp_arrange(Evas_Object *o){
	int basex, y, w;
	int dx,ow;

	state ++;

	evas_object_geometry_get(bg, &basex, NULL, &w, NULL);
	evas_object_geometry_get(text, NULL, &y, &ow, NULL);
	dx = w - ow;

	if (state >= dx) state = 0;

	evas_object_move(text, basex + state, y);
	return;
}

static void
smart_add(Evas_Object *obj){
	Evas *e;

	if (!obj) return;

	e = evas_object_evas_get(obj);

	evas_object_resize(obj, 100, 20);

	bg = evas_object_rectangle_add(e);
	evas_object_resize(bg,100,20);
	evas_object_color_set(bg, 100,180,110,210);
	evas_object_smart_member_add(bg, obj);
	evas_object_show(bg);

	text = evas_object_rectangle_add(e);
	evas_object_resize(text,80,16);
	evas_object_color_set(text, 180,10,110,255);
	evas_object_smart_member_add(text, obj);
	evas_object_show(text);

	evas_object_show(obj);
}

static void
smart_move(Evas_Object *obj, int x, int y){
	evas_object_move(bg, x, y);
	evas_object_move(text, x + 10, y + 2);
}
