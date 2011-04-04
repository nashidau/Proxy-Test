#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#include "smartproxy.h"

#define streq(a,b) (strcmp((a),(b)) == 0)

enum {
	MAGNIFIER_SCALE = 3,
	MAGNIFIER_WIDTH = 150,
	MAGNIFIER_HEIGHT = 150,
	WINDOW_WIDTH = 320,
	WINDOW_HEIGHT = 480,
	IMAGE_PADDING = 0,
};

struct imageupdate {
	Evas_Object *obj;
	int cur, max;
	const char **imagelist;
	Evas_Object *proxy;
};

static Evas_Object *label_add(Evas *, int x, int y, const char *fmt, bool anim);
static Evas_Object *textblock_add(Evas *e, int x, int y);
static void key_down(void *, Evas *, Evas_Object *, void *);
static void flip_map(Evas_Object *o);
static void zoom_map(Evas_Object *o);

static Eina_Bool label_move(void *ov);
static Eina_Bool image_next(void *ov);
static Eina_Bool smart_animate(void *smart);

static Eina_Bool _edje_load_or_show_error(Evas_Object *edje, const char *file, const char *group);

static bool visible = true;
static Eina_List *labels;

static const char *lucases[] = {
//	"lucasyawn.jpg",
//	"lucasstamp.jpg",
//	"lucastractor.jpg",
//	"lucasscarf.jpg"


//	"img01.jpg",
	"img02.jpg"
};
#define N_LUCAS ((int)(sizeof(lucases)/sizeof(lucases[0])))

static void
_mouse_down_cb(void *data, Evas *e, Evas_Object *o, void *event_info)
{
	Evas_Event_Mouse_Down *ev = event_info;
	Evas_Object *obj = (Evas_Object *)data;
	Evas_Object *mgf = evas_object_data_get(obj, "clip");
	Evas_Coord x, y;
	Evas_Coord px,py;

	printf("MOUSE: down @ %4i %4i\n", ev->canvas.x, ev->canvas.y);
	if (ev->button != 1) return;

	x = ev->canvas.x;
	y = ev->canvas.y;

	px = x - MAGNIFIER_WIDTH / 2;
	py = y - MAGNIFIER_HEIGHT / 2;

	if (px < 0) px = 0;
	if (py < 0) py = 0;
	if (px + MAGNIFIER_WIDTH > WINDOW_WIDTH)
		px = WINDOW_WIDTH - MAGNIFIER_WIDTH;
	if (py + MAGNIFIER_HEIGHT > WINDOW_HEIGHT)
		py = WINDOW_HEIGHT - MAGNIFIER_HEIGHT;

	evas_object_move(mgf, px, py);
	evas_object_move(obj, (MAGNIFIER_SCALE - 1) * -x + IMAGE_PADDING, (MAGNIFIER_SCALE - 1) * -y + IMAGE_PADDING);
	evas_object_show(mgf);
}

static void
_mouse_up_cb(void *data, Evas *e, Evas_Object *o, void *event_info)
{
	Evas_Event_Mouse_Up *ev = event_info;
	Evas_Object *obj = (Evas_Object *)data;
	Evas_Object *mgf = evas_object_data_get(obj, "clip");

	printf("MOUSE: up   @ %4i %4i\n", ev->canvas.x, ev->canvas.y);
	if (ev->button != 1) return;

	evas_object_hide(mgf);
}

static void
_mouse_move_cb(void *data, Evas *e, Evas_Object *o, void *event_info)
{
	Evas_Event_Mouse_Move *ev = event_info;
	Evas_Object *obj = (Evas_Object *)data;
	Evas_Object *mgf = evas_object_data_get(obj, "clip");
	Evas_Coord x, y;
	Evas_Coord px,py;

	printf("MOUSE: move @ %4i %4i buttons : %d \n", ev->cur.canvas.x, ev->cur.canvas.y, ev->buttons);

	if (ev->buttons != 1) return;

	x = ev->cur.canvas.x;
	y = ev->cur.canvas.y;

	px = x - MAGNIFIER_WIDTH / 2;
	py = y - MAGNIFIER_HEIGHT / 2;

	if (px < 0) px = 0;
	if (py < 0) py = 0;
	if (px + MAGNIFIER_WIDTH > WINDOW_WIDTH)
		px = WINDOW_WIDTH - MAGNIFIER_WIDTH;
	if (py + MAGNIFIER_HEIGHT > WINDOW_HEIGHT)
		py = WINDOW_HEIGHT - MAGNIFIER_HEIGHT;

	evas_object_move(mgf, px, py);
	evas_object_move(obj, (MAGNIFIER_SCALE - 1) * -x + IMAGE_PADDING, (MAGNIFIER_SCALE - 1) * -y + IMAGE_PADDING);
}

int
main(int argc, char **argv){
	Ecore_Evas *ee = NULL;
	Evas *e;
	Evas_Object *bg, *img, *proxy, *mgf;
	Evas_Object *r = NULL;
	bool rv;
	int w,h;
	struct imageupdate *iu;
	int minw,minh,maxw,maxh;
	int usetext = 0;

	ecore_init();
	ecore_evas_init();

	ee = ecore_evas_gl_x11_new(NULL, 0, 0, 0,
			WINDOW_WIDTH,WINDOW_HEIGHT);

	if (argv[1]) usetext = 1;

	if (!ee){
		printf("Unable to create ecore evas\n");
		return 1;
	}

	e = ecore_evas_get(ee);
	if (!e){
		printf("Unable to get 'e'\n");
		return 1;
	}

	bg = evas_object_rectangle_add(e);
	evas_object_color_set(bg, 200,200,200,255);
	/* FIXME: Scale to window size */
	evas_object_resize(bg, WINDOW_WIDTH, WINDOW_HEIGHT);
	evas_object_event_callback_add(bg, EVAS_CALLBACK_KEY_DOWN,
			key_down, NULL);
	evas_object_focus_set(bg,true);
	evas_object_show(bg);


	//label_add(e,10,0,"Source",false);
	if (!usetext){
		img = evas_object_image_filled_add(e);
		evas_object_image_file_set(img, lucases[0], NULL);
		evas_object_image_size_get(img, &w, &h);
		evas_object_resize(img, w, h);
		evas_object_move(img, IMAGE_PADDING, IMAGE_PADDING);
		evas_object_show(img);
	} else {

		img = textblock_add(e, 0, 0);
		evas_object_geometry_get(img, NULL, NULL, &w, &h);

		/* add a background for 'zooming' */
		r = evas_object_rectangle_add(e);
		evas_object_resize(r, WINDOW_WIDTH, WINDOW_HEIGHT);
		/* Same as background */
		evas_object_color_set(r, 200,200,200,255);
		/* Semi transparent */
		evas_object_color_set(r, 140,140,156,200);
		evas_object_show(r);
	}

	mgf = evas_object_image_filled_add(e);
	evas_object_image_file_set(mgf, "magnifier.png", NULL);
	evas_object_resize(mgf, MAGNIFIER_WIDTH, MAGNIFIER_HEIGHT);
	evas_object_move(mgf, 10,10);
	evas_object_layer_set(mgf,100);
	evas_object_pass_events_set(mgf,true);

	if (r) evas_object_clip_set(r,mgf);

	proxy = evas_object_image_add(e);
	evas_object_image_source_set(proxy, img);
	evas_object_show(proxy);
	evas_object_resize(proxy, w * MAGNIFIER_SCALE, h * MAGNIFIER_SCALE);
	evas_object_image_fill_set(proxy, 0,0,w * MAGNIFIER_SCALE, h * MAGNIFIER_SCALE);
	evas_object_move(proxy, 0, 10);
	evas_object_color_set(proxy, 255,255,255,255);
//	evas_object_clip_set(proxy, mgf);
	evas_object_data_set(proxy, "clip", mgf);
	evas_object_pass_events_set(proxy,true);

	evas_object_event_callback_add(img, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, proxy);
	evas_object_event_callback_add(img, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, proxy);
	evas_object_event_callback_add(img, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, proxy);

#if 0
	label_add(e,10,h + 20, "Reflected Proxy",false);
	proxy = evas_object_proxy_add(e);
	evas_object_proxy_source_set(proxy, img);
	evas_object_resize(proxy, w, h);
	evas_object_move(proxy, 10, 30+h);
	evas_object_show(proxy);
	flip_map(proxy);

	label_add(e,20+w,h+20,"Squish Proxy",false);
	proxy = evas_object_proxy_add(e);
	evas_object_proxy_source_set(proxy, img);
	evas_object_resize(proxy, w, h / 2);
	evas_object_move(proxy, 20+w, 30+h);
	evas_object_show(proxy);



	/* Proxy a label */
	img = label_add(e, 300, 10, "Label Source ",true);
	evas_object_geometry_get(img, NULL, NULL, &w, &h);
	proxy = evas_object_proxy_add(e);
	evas_object_proxy_source_set(proxy, img);
	evas_object_resize(proxy, w, h);
	evas_object_move(proxy, 300, 10 + h + 3);
	evas_object_show(proxy);
	flip_map(proxy);

	label_add(e, 440, 10, "Squish Label",false);
	proxy = evas_object_proxy_add(e);
	evas_object_proxy_source_set(proxy, img);
	evas_object_resize(proxy, w, h / 2);
	evas_object_move(proxy, 440, 10 + h + 3);
	evas_object_show(proxy);

	label_add(e, 440, 60, "Stretch Label",false);
	proxy = evas_object_proxy_add(e);
	evas_object_proxy_source_set(proxy, img);
	evas_object_resize(proxy, w, h * 3);
	evas_object_move(proxy, 440, 60 + h + 3);
	evas_object_show(proxy);

	img = label_add(e, 400, 120, "Zoomy Text!", false);
	proxy = evas_object_proxy_add(e);
	evas_object_proxy_source_set(proxy, img);
	evas_object_resize(proxy, w, h);
	evas_object_move(proxy, 350, 150);
	zoom_map(proxy);
	evas_object_show(proxy);

	/* Proxy a text block */
	img = textblock_add(e, 10, 200);

	evas_object_geometry_get(img, NULL, NULL, &w, &h);
	proxy = evas_object_proxy_add(e);
	evas_object_proxy_source_set(proxy, img);
	evas_object_resize(proxy, w, h);
	evas_object_move(proxy, 10, 320);
	evas_object_show(proxy);
	flip_map(proxy);

	/* The 'smart' object */
	img = sp_add(e);
	evas_object_move(img, 300,200);
	evas_object_resize(img, 100, 20);
	ecore_timer_add(0.05, smart_animate, img);
	w = 100;
	h = 20;
	proxy = evas_object_proxy_add(e);
	evas_object_proxy_source_set(proxy, img);
	evas_object_resize(proxy, w, h);
	evas_object_move(proxy, 300, 240);
	evas_object_show(proxy);
	flip_map(proxy);


	label_add(e, 300,90, "Edje File", false);
	img = edje_object_add(e);
	if (!_edje_load_or_show_error(img, "basic.edj", "proxytest")){
		  evas_object_del(img);
	}

	evas_object_resize(img,220,200);
	evas_object_move(img,300,100);
	evas_object_show(img);
	edje_object_size_max_get(img, &maxw, &maxh);
	edje_object_size_min_get(img, &minw, &minh);
	if ((minw <= 0) && (minh <= 0))
		edje_object_size_min_calc(img, &minw, &minh);
	evas_object_size_hint_max_set(img, maxw, maxh);
	evas_object_size_hint_min_set(img, minw, minh);
	evas_object_size_hint_weight_set(img,
			EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(img, EVAS_HINT_FILL, EVAS_HINT_FILL);
#endif /* The edje file */



//#endif
	ecore_evas_show(ee);
	ecore_main_loop_begin();

	return 0;
}


/**
 * Add a text object to the screen.
 *
 * @todo: Make fmt a printf fmt string
 */
Evas_Object *
label_add(Evas *e, int x, int y, const char *fmt, bool anim){
	Evas_Object *o;
	Ecore_Timer *timer;

	if (!e) return NULL;

	o = evas_object_text_add(e);
	evas_object_text_font_set(o, "Vera", 15);
	evas_object_text_style_set(o, EVAS_TEXT_STYLE_SOFT_OUTLINE);
	evas_object_color_set(o, 0,0,0,255);
	evas_object_text_outline_color_set(o,255,255,255,255);

	evas_object_layer_set(o, 1);
	evas_object_text_text_set(o,fmt);
	evas_object_move(o,x,y);
	if (visible) evas_object_show(o);

	labels = eina_list_append(labels, o);

	if (!anim) return o;

	timer = ecore_timer_add(0.1, label_move, o);

	return o;
}

static Eina_Bool
label_move(void *ov){
	char *str,t;
	int len;

	str = strdup(evas_object_text_text_get(ov));
	t = *str;
	len = strlen(str);
	memmove(str,str+1,len - 1);
	str[len - 1] = t;
	evas_object_text_text_set(ov, str);
	free(str);

	return true;
}

static Eina_Bool
image_next(void *info){
	struct imageupdate *iu = info;
	if (!info) return false;

	iu->cur ++;
	if (iu->cur >= iu->max) iu->cur = 0;

	evas_object_image_file_set(iu->obj, iu->imagelist[iu->cur], NULL);

	if (iu->proxy){
		int x,y;
		evas_object_geometry_get(iu->proxy, &x, &y, NULL, NULL);
		evas_object_move(iu->proxy, y, x);
	}

	return true;
}

static Eina_Bool
smart_animate(void *smart){
	sp_arrange(smart);
	return true;
}

Evas_Object *
textblock_add(Evas *e, int x, int y){
	Evas_Object *o;
	Evas_Textblock_Style *st;

	o = evas_object_textblock_add(e);

	st = evas_textblock_style_new();
	evas_textblock_style_set(st,
	      "DEFAULT='font=Vera,Kochi font_size=8 align=left color=#000000 wrap=word'"
	      "center='+ font=Vera,Kochi font_size=10 align=center'"
	      "/center='- \n'"
	      "right='+ font=Vera,Kochi font_size=10 align=right'"
	      "/right='- \n'"
	      "blockquote='+ left_margin=+24 right_margin=+24 font=Vera,Kochi font_size=10 align=left'"
	      "h1='+ font_size=20'"
	      "red='+ color=#ff0000'"
	      "p='+ font=Vera,Kochi font_size=10 align=left'"
	      "/p='- \n'"
	      "br='\n'"
	      "tab='\t'"
	      );
   evas_object_textblock_style_set(o, st);
   evas_textblock_style_free(st);
   evas_object_textblock_clear(o);
	evas_object_resize(o, WINDOW_WIDTH, WINDOW_HEIGHT/2);
evas_object_textblock_text_markup_set
     (o,
      "<center><h1>Title</h1></center><br>"
      "<p><tab>A pragraph here <red>red text</red> and stuff.</p>"
      "<p>And escaping &lt; and &gt; as well as &amp; as <h1>normal.</h1></p>"
      "<p>If you want a newline use &lt;br&gt;<br>woo a new line!</p>"
      "<right>Right "
      "<style=outline color=#fff outline_color=#000>aligned</> "
      "<style=shadow shadow_color=#fff8>text</> "
      "<style=soft_shadow shadow_color=#0002>should</> "
      "<style=glow color=#fff glow2_color=#fe87 glow_color=#f214 >go here</> "
      "<style=far_shadow shadow_color=#0005>as it is</> "
      "<style=outline_shadow color=#fff outline_color=#8228 shadow_color=#005>within</> "
      "<center><h1>Title</h1></center><br>"
      "<p><tab>A pragraph here <red>red text</red> and stuff.</p>"
      "<p>And escaping &lt; and &gt; as well as &amp; as <h1>normal.</h1></p>"
      "<p>If you want a newline use &lt;br&gt;<br>woo a new line!</p>"
      "<right>Right "
      "<style=outline color=#fff outline_color=#000>aligned</> "
      "<style=shadow shadow_color=#fff8>text</> "
      "<style=soft_shadow shadow_color=#0002>should</> "
      "<style=glow color=#fff glow2_color=#fe87 glow_color=#f214 >go here</> "
      "<style=far_shadow shadow_color=#0005>as it is</> "
      "<style=outline_shadow color=#fff outline_color=#8228 shadow_color=#005>within</> "
     );

	//evas_object_move(o,x,y);
	evas_object_show(o);

	return o;
}

static void
flip_map(Evas_Object *o){
	int x, y, w, h;
	int xx, z;
	Evas_Map *m;

	evas_object_geometry_get(o, &x, &y, &w, &h);

	m = evas_map_new(4);
	xx = x + w;
	z = 0;

	evas_map_point_coord_set   (m, 0, x, y, -z);
        evas_map_point_image_uv_set(m, 0, 0, h);
        evas_map_point_color_set   (m, 0, 128, 128, 128, 128);

        evas_map_point_coord_set   (m, 1, xx, y, -z);
        evas_map_point_image_uv_set(m, 1, w, h);
        evas_map_point_color_set   (m, 1, 128, 128, 128, 128);

        evas_map_point_coord_set   (m, 2, xx, y + h, -z);
        evas_map_point_image_uv_set(m, 2, w, 0);
        evas_map_point_color_set   (m, 2, 0, 0, 0, 0);

        evas_map_point_coord_set   (m, 3, x, y + h, -z);
        evas_map_point_image_uv_set(m, 3, 0, 0);
        evas_map_point_color_set   (m, 3, 0, 0, 0, 0);

	evas_object_map_enable_set(o, 1);
        evas_object_map_set(o, m);
	evas_map_free(m);
}


static void
zoom_map(Evas_Object *o){
	int x, y, w, h;
	Evas_Map *m;

	evas_object_geometry_get(o, &x, &y, &w, &h);
	h = h * 3;

	m = evas_map_new(4);

	evas_map_point_coord_set   (m, 0, x + w/2, y, 0);
        evas_map_point_image_uv_set(m, 0, 0, 0);
        evas_map_point_color_set   (m, 0, 255, 255, 255, 255);

        evas_map_point_coord_set   (m, 1, x + w + w/2, y, 0);
        evas_map_point_image_uv_set(m, 1, w, 0);
        evas_map_point_color_set   (m, 1, 255, 255, 255, 255);

        evas_map_point_coord_set   (m, 2, x + 2 * w, y + h, 0);
        evas_map_point_image_uv_set(m, 2, w, h);
        evas_map_point_color_set   (m, 2, 255, 255, 255, 255);

        evas_map_point_coord_set   (m, 3, x, y + h, 0);
        evas_map_point_image_uv_set(m, 3, 0, h);
        evas_map_point_color_set   (m, 3, 255, 255, 255, 255);

	evas_object_map_enable_set(o, 1);
        evas_object_map_set(o, m);
	evas_map_free(m);
}


void
key_down(void *data, Evas *e, Evas_Object *obj, void *ev){
	Evas_Event_Key_Down *key = ev;
	Eina_List *l;

	if (!ev) return;

	if (streq(key->keyname,"Space") || streq(key->keyname,"space")){
		visible = !visible;
		for (l = labels ; l ; l = l->next){
			if (visible)
				evas_object_show(l->data);
			else
				evas_object_hide(l->data);
		}
	} else if (streq(key->keyname,"q") || streq(key->keyname, "Escape")){
		ecore_main_loop_quit();
	}
}

/* straight from edje player */
static Eina_Bool
_edje_load_or_show_error(Evas_Object *edje, const char *file, const char *group)
{
   const char *errmsg;
   int err;

   if (edje_object_file_set(edje, file, group))
     {
        evas_object_focus_set(edje, EINA_TRUE);
        return EINA_TRUE;
     }

   err = edje_object_load_error_get(edje);
   errmsg = edje_load_error_str(err);
   fprintf(stderr, "ERROR: could not load edje file '%s', group '%s': %s\n",
	   file, group, errmsg);
   return EINA_FALSE;
}
